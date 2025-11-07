class Buffer
  # Simple structs for clarity
  Diagnostic = Struct.new(:x0, :y0, :x1, :y1, :message)
  Highlight  = Struct.new(:x0, :y0, :x1, :y1, :fg, :bg)
  VirtualText = Struct.new(:x, :y, :lines)
  Cursor = Struct.new(:x, :y)

  attr_accessor :text, :cursor, :selection_start,
                :diagnostics, :highlights, :virtual_texts

  def initialize(initial_text = "")
    @text = initial_text
    @cursor = Cursor.new(0, 0)
    @selection_start = Cursor.new(0, 0)
    @diagnostics = []
    @highlights = []
    @virtual_texts = []
  end

  # Utility methods
  def lines
    @text.split("\n")
  end

  def line_count
    lines.size
  end

  def line(y)
    lines[y] || ""
  end

  def insert(x, y, str)
    current_line = lines[y] || ""
    before = current_line[0...x] || ""
    after  = current_line[x..-1] || ""
    lines_arr = lines
    lines_arr[y] = before + str + after
    @text = lines_arr.join("\n")
  end

  def erase(x0, y0, x1, y1)
    lines_arr = lines
    if y0 == y1
      line = lines_arr[y0] || ""
      lines_arr[y0] = line[0...x0] + line[x1..-1].to_s
    else
      first = lines_arr[y0][0...x0]
      last  = lines_arr[y1][x1..-1].to_s
      lines_arr[y0..y1] = [first + last]
    end
    @text = lines_arr.join("\n")
  end

  # Add overlays
  def add_diagnostic(x0, y0, x1, y1, message)
    @diagnostics << Diagnostic.new(x0, y0, x1, y1, message)
  end

  def add_highlight(x0, y0, x1, y1, fg: 0xFFFFFF, bg: 0x000000)
    @highlights << Highlight.new(x0, y0, x1, y1, fg, bg)
  end

  def add_virtual_text(x, y, lines)
    @virtual_texts << VirtualText.new(x, y, lines)
  end

  def render()
  end
end
