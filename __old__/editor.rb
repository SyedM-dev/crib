class Editor
  attr_accessor :text, :cursor, :selection_start, :filename

  def initialize(filename, x, y, width, height)
    contents = File.exist?(filename) ? File.read(filename) : "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
    @text = contents.grapheme_clusters
    @cursor = @text.size
    @selection_start = 0
    @scroll = 0
    @x = x
    @y = y
    @width = width
    @height = height
    @filename = filename
    @folds = []
    @highlights = Array.new(@text.size)
    @highlight_generation = 0
    @tree = nil
    @num_width = (@text.count("\n") + 1).to_s.size + 3

    @parser = C.ts_parser_new
    @lang_ts = C.tree_sitter_ruby
    C.ts_parser_set_language @parser, @lang_ts
    @cached_tree = nil
    C.load_query "/home/syed/main/crib/grammar/ruby.scm"

    highlight!

    rebuild_folded_lines!
  end

  def save!
    File.write @filename, @text.join
  end

  def highlight!
    gen = @highlight_generation
    src = @text.join
    text_bytes = []
    byte_offset = 0
    @text.each do |cluster|
      text_bytes << byte_offset
      byte_offset += cluster.bytesize
    end
    text_bytes << byte_offset
    line_map = []
    line = 0
    @text.each_with_index do |ch, i|
      line_map[i] = line
      line += 1 if ch == "\n"
    end
    # TODO: add ts_tree_edit calls in erase/insert
    count_ptr = FFI::MemoryPointer.new :uint32
    @cached_tree = C.ts_parser_parse_string @parser, nil, src, src.bytesize
    root = C.ts_tree_root_node @cached_tree
    ptr = C.ts_collect_tokens root, count_ptr, @lang_ts, src
    count = count_ptr.read_uint32
    ints  = ptr.read_array_of_uint32 count
    C.free_tokens ptr
    count_ptr.free
    C.ts_tree_delete @cached_tree
    return if gen != @highlight_generation
    @highlight_generation = 0
    @highlights = Array.new(@text.size)
    done_folds = []
    ints.each_slice(3).map do |start_byte, end_byte, sym_i|
      start_cluster = byte_to_cluster start_byte, text_bytes
      end_cluster   = byte_to_cluster end_byte - 1, text_bytes
      next if start_cluster.nil? || end_cluster.nil?
      sym = TS_SYMBOL_MAP[sym_i]
      if sym == "@fold"
        start_row = line_map[start_cluster]
        end_row   = line_map[end_cluster]
        next if start_row == end_row
        hash_value = Zlib.crc32 @text[start_cluster...end_cluster].join
        done_folds << hash_value
        if (f = @folds.find { |f| f.crc32 == hash_value })
          f.start = start_row
          f.end = end_row
        else
          @folds << Fold.new(start_row, end_row, false, hash_value)
        end
        next
      end
      hl  = TS_RUBY[sym]
      next unless hl
      (start_cluster..end_cluster).each do |i|
        @highlights[i] = hl if @highlights[i].nil? || @highlights[i].priority < hl.priority
      end
    end
    @folds.select! { |f| done_folds.include? f.crc32 }
    rebuild_folded_lines!
  end

  def byte_to_cluster(byte, byte_starts)
    left = 0
    right = byte_starts.size - 2
    while left <= right
      mid = (left + right) / 2
      if byte_starts[mid] <= byte && byte < byte_starts[mid + 1]
        return mid
      elsif byte < byte_starts[mid]
        right = mid - 1
      else
        left = mid + 1
      end
    end
    nil
  end

  def erase(x0, x1)
    return if x0 < 0 || x1 < x0
    lines_deleted = @text[x0...x1].count "\n"
    @text[x0...x1] = []
    @highlights[x0...x1] = []
    @highlight_generation += 1
    @folds.map! do |f|
      if f.start >= @text[...x0].count("\n")
        f.start -= lines_deleted
        f.end -= lines_deleted
      end
      f
    end
    rebuild_folded_lines!
    @num_width = (@text.count("\n") + 1).to_s.size + 3
  end

  def insert(x, str)
    @text.insert x, *str.grapheme_clusters
    @highlights.insert x, *Array.new(str.grapheme_clusters.size)
    lines_added = str.count "\n"
    @folds.map! do |f|
      if f.start >= @text[...x].count("\n")
        f.start += lines_added
        f.end += lines_added
      end
      f
    end
    rebuild_folded_lines!
    @highlight_generation += 1
    @num_width = (@text.count("\n") + 1).to_s.size + 3
  end

  def move_up!
    precursor_text = @text[...@cursor]
    last_newline = precursor_text.rindex "\n"
    return unless last_newline
    @preferred_col ||= @cursor - last_newline
    prev_start = precursor_text[...last_newline].rindex("\n") || -1
    @cursor = [
      prev_start + @preferred_col,
      @text[prev_start + 1..].index("\n")&.+(prev_start + 1) || @text.size
    ].min
  end

  def move_down!
    next_newline = @text[@cursor..].index("\n")&.+(@cursor)
    return unless next_newline
    @preferred_col ||= (@cursor - (@text[...@cursor].rindex("\n") || -1)).abs
    @cursor = [
      next_newline + @preferred_col,
      @text[next_newline + 1..].index("\n")&.+(next_newline + 1) || @text.size
    ].min
  end

  def move_left!
    @cursor = [@cursor - 1, 0].max
    @preferred_col = nil
  end

  def move_right!
    @cursor = [@cursor + 1, @text.size].min
    @preferred_col = nil
  end

  def cursor_valid?
    !@folded_lines.include? @text[0...@cursor].count("\n")
  end

  def handle_event(event)
    return if event.nil?
    case KEY_TYPE[event[:key_type]]
    when :char
      ch = event[:c].chr
      ch = "\n" if ch == "\r"
      if ch == "'"
        cr = @text[0...@cursor].count("\n")
        @folds.find { |f| f.active = !f.active if f.start <= cr && f.end >= cr }
        rebuild_folded_lines!
      elsif ch == ctrl_key('s').chr
        save!
      elsif ch =~ /[[:print:]]|\n/
        insert @cursor, ch
        @cursor += 1
      elsif ch == "\b" || ch == "\x7f"
        erase @cursor - 1, @cursor
        @cursor = [@cursor - 1, 0].max
      end
      @preferred_col = nil
    when :special
      return unless MODIFIER[event[:special_modifier]] == :none
      case SPECIAL_KEY[event[:special_key]]
      when :up
        move_up!
        move_up! until cursor_valid?
      when :down
        move_down!
        move_down! until cursor_valid?
      when :left
        move_left!
        move_left! until cursor_valid?
      when :right
        move_right!
        move_right! until cursor_valid?
      when :delete
        erase @cursor, @cursor + 1
        @cursor = [@cursor, @text.size].min
        @preferred_col = nil
      end
    end
    move_right! until cursor_valid?
    adjust_scroll!
  end

  def adjust_scroll!
    cr = cursor_row_visual
    start = @scroll
    last = @scroll + @height - 1
    if cr < start
      @scroll = cr
    elsif cr > last
      @scroll = cr - @height + 1
    end
  end

  def cursor_row_visual
    row = col = 0
    nc = 0
    @text.each_with_index do |ch, i|
      return row if i == @cursor
      if @folded_lines.include? nc
        nc += 1 if ch == "\n"
        next
      end
      if ch == "\n"
        row += 1
        nc += 1
        col = 0
        next
      end
      w = C.display_width(ch)
      if col + w > @width - @num_width
        row += 1
        col = 0
      end
      col += w
    end
    row
  end

  def rebuild_folded_lines!
    @folded_lines = Set.new
    @folds.each do |f|
      (f.start + 1..f.end).each { |line_num| @folded_lines.add(line_num) } if f.active
    end
  end

  def render
    (0...@height).each { |h| (0...@width).each { |w|
      C.update @y + h, @x + w, ' ', 0x000000, 0x000000, 0
    } }
    row = col = nc = 0
    cursor_row = cursor_col = nil
    @text.each_with_index do |ch, i|
      if @folded_lines.include? nc
        nc += 1 if ch == "\n"
        next
      end
      if i == @cursor
        cursor_row = row
        cursor_col = col
      end
      if col == 0
        color = 0x666666
        color = 0xFFFFFF if cursor_row == row
        num = (nc + 1).to_s.rjust @num_width - 1, '👪👩‍💻👨🏿‍🚀🏳️‍🌈🔥🛠️🌍🥶✅❌💡⏰🚀'#ddasasdasda
        num.each_char.with_index do |c, j|
          C.update @y + row - @scroll, @x + j, c, color, 0x000000, 0
        end
        if (fold = @folds.find { |f| nc == f.start })
          icon = fold.active ? "" : "" # "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : "" "" : ""
          C.update @y + row - @scroll, @x, icon, 0xFF0000, 0x000000, 0
        end
      end
      if ch == "\n"
        row += 1
        nc += 1
        col = 0
        next
      end
      w = C.display_width(ch)
      if col + w > @width - @num_width
        row += 1
        col = 0
      end
      col += w
      next if row < @scroll
      break if row - @scroll >= @height
      fg = 0xFFFFFF
      bg = 0x000000
      fl = 0
      if (h = @highlights[i])
        fg = h.color_fg
        bg = h.color_bg
        fl = h.flags
      end
      C.update @y + row - @scroll, @x + @num_width + col - w, ch, fg, bg, fl
    end
    if @text[-1] == "\n" || @text.size == 0
      num = (nc + 1).to_s.rjust @num_width - 1, ' '
      color = 0x666666
      color = 0xFFFFFF if (cursor_row || row) == row
      num.each_char.with_index do |c, j|
        C.update @y + row - @scroll, @x + j, c, color, 0x000000, 0
      end
    end
    C.set_cursor @y - @scroll + (cursor_row || row), @x + @num_width + (cursor_col || col), 1
  end
end
