module FileManager
  @hidden = true

  module_function

  def start(x, y, width, height)
    @hidden = false
    @x = x
    @y = y
    @width = width
    @height = height
  end

  def toggle!
    @hidden = !@hidden
  end

  def render
    return if @hidden
    (0...@height).each { |h| (0...@width).each { |w|
      C.update @y + h, @x + w, ' ', 0x000000, 0x000000, 0
    } }
    files = $folder.children
    files.each_with_index do |f, i|
      f.each_char.with_index do |c, j|
        C.update @y + i, @x + j, c, 0xFFFFFF, 0x000000, 0
      end
    end
  end
end
