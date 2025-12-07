module IDE
  FM_WIDTH = 20

  module_function
  
  def start
    @editors = {}
    @u_c = 0
    editor = Editor.new (ARGV[0] || ""), FM_WIDTH, 0, $cols - FM_WIDTH, $rows
    @editors[editor.filename || "untitled #{@u_c += 1}"] = editor
    @selected_editor = @editors.keys.first
    @focus = :editor
    FileManager.start 0, 0, FM_WIDTH, $rows
  end

  def handle_event(event)
    if @focus == :editor
      @editors[@selected_editor].handle_event event if @editors.key? @selected_editor
    elsif @focus == :file_manager
      # TODO
    end
  end

  def render
    @editors[@selected_editor].render if @editors.key? @selected_editor
    FileManager.render
  end

  def work!
    @editors[@selected_editor].highlight! if @editors.key? @selected_editor
  end

  def close
    # TODO
  end
end
