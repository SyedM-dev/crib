query = File.read "/home/syed/main/crib/grammar/ruby.scm"

raw = query.scan(/@[a-zA-Z0-9_.]+/)

seen = {}
ordered = []

raw.each do |c|
  next if seen[c]
  seen[c] = true
  ordered << c
end

TS_SYMBOL_MAP = ordered.freeze

spawn "echo \"#{TS_SYMBOL_MAP.sort.join "\n"}\" > /tmp/gg"

TS_RUBY = {
  "@string.special.symbol" => Highlight.new(0xbd9ae6, 0x000000, CF_NONE, 2),
  "@comment"               => Highlight.new(0xAAAAAA, 0x000000, CF_ITALIC, 1),
  "@boolean.true"          => Highlight.new(0x51eeba, 0x000000, CF_NONE, 1),
  "@boolean.false"         => Highlight.new(0xee513a, 0x000000, CF_NONE, 1),
  "@constant.nil"          => Highlight.new(0xee8757, 0x000000, CF_NONE, 1),
  "@constant"              => Highlight.new(0xebda8c, 0x000000, CF_NONE, 1),
  "@number"                => Highlight.new(0xebda8c, 0x000000, CF_NONE, 2),
  "@number.float"          => Highlight.new(0xebda8c, 0x000000, CF_NONE, 2),
  "@constant.builtin"      => Highlight.new(0xfbb152, 0x000000, CF_NONE, 2),
  "@punctuation.bracket"   => Highlight.new(0xbd9ae6, 0x000000, CF_NONE, 1),
  "@operator.ligature"     => Highlight.new(0xffffff, 0x000000, CF_ITALIC, 1),
  "@operator"              => Highlight.new(0xffffff, 0x000000, CF_NONE, 1),
  "@punctuation.delimiter" => Highlight.new(0xbd9ae6, 0x000000, CF_NONE, 1),
  "@punctuation.special"   => Highlight.new(0xe6a24c, 0x000000, CF_NONE, 1),
  "@function"              => Highlight.new(0xaad84c, 0x000000, CF_NONE, 1),
  "@function.builtin"      => Highlight.new(0xaad84c, 0xFF0000, CF_NONE, 1),
  "@keyword.import"        => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@function.call"         => Highlight.new(0xff5689, 0x000000, CF_NONE, 1),

  "@keyword"               => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@keyword.conditional"   => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@keyword.control"       => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@keyword.directive"     => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@keyword.exception"     => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@keyword.function"      => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@keyword.operator"      => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@keyword.repeat"        => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@keyword.return"        => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@keyword.type"          => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@label"                 => Highlight.new(0xfbb152, 0x000000, CF_NONE, 1),
  "@string"                => Highlight.new(0xaad84c, 0x000000, CF_NONE, 1),
  "@string.escape"         => Highlight.new(0xe6a24c, 0x000000, CF_NONE, 2),
  "@string.regexp"         => Highlight.new(0xe6a24c, 0x000000, CF_NONE, 2),
  "@type"                  => Highlight.new(0xaad84c, 0x000000, CF_NONE, 1),
  "@variable"              => Highlight.new(0xffffff, 0x000000, CF_NONE, 1),
  "@variable.builtin"      => Highlight.new(0xffffff, 0x000000, CF_NONE, 1),
  "@variable.member"       => Highlight.new(0xffffff, 0x000000, CF_NONE, 1),
  "@variable.parameter"    => Highlight.new(0xffffff, 0x000000, CF_NONE, 1),
}
