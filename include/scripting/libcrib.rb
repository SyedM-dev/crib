module C
  @lsp_config = {
    "clangd" => [
      "--background-index",
      "--clang-tidy",
      "--completion-style=detailed",
      "--header-insertion=never",
      "--pch-storage=memory",
      "--limit-results=50",
      "--log=error"
    ],
    "ruby-lsp" => [],
    "solargraph" => ["stdio"],
    "bash-language-server" => ["start"],
    "vscode-css-language-server" => ["--stdio"],
    "vscode-json-language-server" => ["--stdio"],
    "fish-lsp" => ["start"],
    "gopls" => ["serve"],
    "haskell-language-server" => ["lsp"],
    "emmet-language-server" => ["--stdio"],
    "typescript-language-server" => ["--stdio"],
    "lua-language-server" => [],
    "pyright-langserver" => ["--stdio"],
    "rust-analyzer" => [],
    "intelephense" => ["--stdio"],
    "marksman" => ["server"],
    "nginx-language-server" => [],
    "taplo" => ["lsp", "stdio"],
    "yaml-language-server" => ["--stdio"],
    "sqls" => ["serve"],
    "make-language-server" => [],
    "sql-language-server" => ["up", "--method", "stdio"]
  }
  @languages = {
    c: {
      color: 0x555555,
      symbol: " ",
      extensions: ["c"],
      lsp: "clangd"
    },
    cpp: {
      color: 0x00599C,
      symbol: " ",
      extensions: ["cpp", "cc", "cxx"],
      lsp: "clangd"
    },
    h: {
      color: 0xA8B9CC,
      symbol: " ",
      extensions: ["h", "hpp"],
      lsp: "clangd"
    },
    css: {
      color: 0x36a3d9,
      symbol: " ",
      extensions: ["css"],
      lsp: "vscode-css-language-server"
    },
    fish: {
      color: 0x4d5a5e,
      symbol: " ",
      extensions: ["fish"],
      lsp: "fish-lsp"
    },
    go: {
      color: 0x00add8,
      symbol: " ",
      extensions: ["go"],
      lsp: "gopls"
    },
    gomod: {
      color: 0x00add8,
      symbol: " ",
      extensions: ["mod"],
      lsp: "gopls"
    },
    haskell: {
      color: 0xa074c4,
      symbol: " ",
      extensions: ["hs", "lhs"],
      lsp: "haskell-language-server"
    },
    html: {
      color: 0xef8a91,
      symbol: " ",
      extensions: ["html", "htm"],
      lsp: "emmet-language-server"
    },
    javascript: {
      color: 0xf0df8a,
      symbol: " ",
      extensions: ["js"],
      lsp: "typescript-language-server"
    },
    typescript: {
      color: 0x36a3d9,
      symbol: " ",
      extensions: ["ts"],
      lsp: "typescript-language-server"
    },
    json: {
      color: 0xcbcb41,
      symbol: "{}",
      extensions: ["json"],
      lsp: "vscode-json-language-server"
    },
    jsonc: {
      color: 0xcbcb41,
      symbol: "{}",
      extensions: ["jsonc"],
      lsp: "vscode-json-language-server"
    },
    erb: {
      color: 0x6e1516,
      symbol: " ",
      extensions: ["erb"],
      lsp: "ruby-lsp"
    },
    lua: {
      color: 0x36a3d9,
      symbol: "󰢱 ",
      extensions: ["lua"],
      lsp: "lua-language-server"
    },
    python: {
      color: 0x95e6cb,
      symbol: "󰌠 ",
      extensions: ["py"],
      lsp: "pyright"
    },
    rust: {
      color: 0xdea584,
      symbol: "󱘗 ",
      extensions: ["rs"],
      lsp: "rust-analyzer"
    },
    php: {
      color: 0xa074c4,
      symbol: "󰌟 ",
      extensions: ["php"],
      lsp: "intelephense"
    },
    markdown: {
      color: 0x36a3d9,
      symbol: " ",
      extensions: ["md", "markdown"],
      lsp: "marksman"
    },
    nginx: {
      color: 0x6d8086,
      symbol: " ",
      extensions: ["conf"],
      lsp: "nginx-language-server"
    },
    toml: {
      color: 0x36a3d9,
      symbol: " ",
      extensions: ["toml"],
      lsp: "taplo"
    },
    yaml: {
      color: 0x6d8086,
      symbol: " ",
      extensions: ["yml", "yaml"],
      lsp: "yaml-language-server"
    },
    sql: {
      color: 0xdad8d8,
      symbol: " ",
      extensions: ["sql"],
      lsp: "sqls"
    },
    make: {
      color: 0x4e5c61,
      symbol: " ",
      extensions: ["Makefile", "makefile"],
      lsp: "make-language-server"
    },
    gdscript: {
      color: 0x6d8086,
      symbol: " ",
      extensions: ["gd"]
    },
    man: {
      color: 0xdad8d8,
      symbol: " ",
      extensions: ["man"]
    },
    diff: {
      color: 0xDD4C35,
      symbol: " ",
      extensions: ["diff", "patch"]
    },
    gitattributes: {
      color: 0xF05032,
      symbol: " ",
      extensions: ["gitattributes"]
    },
    gitignore: {
      color: 0xF05032,
      symbol: " ",
      extensions: ["gitignore"]
    },
    regex: {
      color: 0x9E9E9E,
      symbol: ".*",
      extensions: ["regex"]
    },
    ini: {
      color: 0x6d8086,
      symbol: " ",
      extensions: ["ini"]
    },
    ruby: {
      color: 0xff8087,
      symbol: "󰴭 ",
      extensions: ["rb"],
      filenames: ["Gemfile"],
      lsp: "solargraph"
    },
    bash: {
      color: 0x4d5a5e,
      symbol: " ",
      extensions: ["sh"],
      filenames: ["bash_profile", "bashrc"],
      lsp: "bash-language-server"
    }
  }
  @theme = {
    :default => { fg: 0xEEEEEE },
    :shebang => { fg: 0x7DCFFF },
    :error => { fg: 0xEF5168 },
    :comment => { fg: 0xAAAAAA, italic: true },
    :string => { fg: 0xAAD94C },
    :escape => { fg: 0x7DCFFF },
    :interpolation => { fg: 0x7DCFFF },
    :regexp => { fg: 0xD2A6FF },
    :number => { fg: 0xE6C08A },
    :true => { fg: 0x7AE93C },
    :false => { fg: 0xEF5168 },
    :char => { fg: 0xFFAF70 },
    :keyword => { fg: 0xFF8F40 },
    :keywordoperator => { fg: 0xF07178 },
    :operator => { fg: 0xFFFFFF, italic: true },
    :function => { fg: 0xFFAF70 },
    :type => { fg: 0xF07178 },
    :constant => { fg: 0x7DCFFF },
    :variableinstance => { fg: 0x95E6CB },
    :variableglobal => { fg: 0xF07178 },
    :annotation => { fg: 0x7DCFFF },
    :directive => { fg: 0xFF8F40 },
    :label => { fg: 0xD2A6FF },
    :brace1 => { fg: 0xD2A6FF },
    :brace2 => { fg: 0xFFAFAF },
    :brace3 => { fg: 0xFFFF00 },
    :brace4 => { fg: 0x0FFF0F },
    :brace5 => { fg: 0xFF0F0F }
  }
  @line_endings = :auto_unix
  @key_handlers = {}
  @key_binds = {}
  @highlighters = {}
  @b_startup = nil
  @b_shutdown = nil
  @b_bar = lambda do |info|
    # mode, lang_name, warnings, lsp_name, filename, foldername, line, max_line, width
    # puts info.inspect
    mode_color = 0x82AAFF
    mode_symbol = "  "
    case info[:mode]
    when :normal
      mode_color = 0x82AAFF
      mode_symbol = " "
    when :insert
      mode_color = 0xFF8F40
      mode_symbol = "󱓧 "
    when :select
      mode_color = 0x9ADE7A
      mode_symbol = "󱩧 "
    when :runner
      mode_color = 0xFFD700
      mode_symbol = " "
    when :jumper
      mode_color = 0xF29CC3
      mode_symbol = " "
    end
    lang_info = C.languages[info[:lang_name]]
    filename = File.basename(info[:filename])
    starting = " #{mode_symbol} #{info[:mode].to_s.upcase}  #{lang_info[:symbol]}#{filename}"
    highlights = []
    highlights << { fg: 0x0b0e14, bg: mode_color, flags: 1 << 1, start: 0, length: 10 }
    highlights << { fg: mode_color, bg: 0x33363c, start: 10, length: 1 }
    highlights << { fg: 0x33363c, bg: 0x24272d, start: 11, length: 1 }
    highlights << { fg: lang_info[:color], bg: 0x24272d, start: 13, length: 2 }
    highlights << { fg: 0xced4df, bg: 0x24272d, start: 15, length: filename.length }
    highlights << { fg: 0x24272d, bg: 0x000000, start: 15 + filename.length, length: 1 }
    return {
      text: starting,
      highlights: highlights
    }
  end

  class << self
    attr_accessor :theme, :lsp_config, :languages,
                  :line_endings, :highlighters
    attr_reader :b_startup, :b_shutdown, :b_extra_highlights, :b_bar

    # def bar=(block)
    #   @b_bar = block
    # end

    def startup(&block)
      @b_startup = block
    end

    def shutdown(&block)
      @b_shutdown = block
    end

    def extra_highlights(&block)
      @b_extra_highlights = block
    end

    def bind(modes, keys = nil, action = nil, &block)
      modes = [modes] unless modes.is_a?(Array)
      if keys.nil?
        app = self
        dsl = Object.new
        dsl.define_singleton_method(:set) do |k, act = nil, &blk|
          app.bind(modes, k, act, &blk)
        end
        dsl.instance_exec(&block) if block_given?
      elsif block_given?
        keys = [keys] unless keys.is_a?(Array)
        modes.each do |mode|
          keys.each do |key|
            @key_handlers[mode] ||= {}
            @key_handlers[mode][key] ||= []
            @key_handlers[mode][key] << block
          end
        end
      elsif action.is_a?(String)
        keys = [keys] unless keys.is_a?(Array)
        modes.each do |mode|
          keys.each do |key|
            @key_binds[mode] ||= {}
            @key_binds[mode][key] ||= []
            @key_binds[mode][key] << action
          end
        end
      end
    end
  end
end
