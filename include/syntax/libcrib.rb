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
      filenames: [],
      mimetypes: ["text/x-c"],
      lsp: "clangd"
    },
    cpp: {
      color: 0x00599C,
      symbol: " ",
      extensions: ["cpp", "cc", "cxx"],
      filenames: [],
      mimetypes: ["text/x-cpp"],
      lsp: "clangd"
    },
    h: {
      color: 0xA8B9CC,
      symbol: " ",
      extensions: ["h", "hpp"],
      filenames: [],
      mimetypes: ["text/x-c-header"],
      lsp: "clangd"
    },
    css: {
      color: 0x36a3d9,
      symbol: " ",
      extensions: ["css"],
      filenames: [],
      mimetypes: ["text/css"],
      lsp: "vscode-css-language-server"
    },
    fish: {
      color: 0x4d5a5e,
      symbol: " ",
      extensions: ["fish"],
      filenames: [],
      mimetypes: ["application/x-fish"],
      lsp: "fish-lsp"
    },
    go: {
      color: 0x00add8,
      symbol: " ",
      extensions: ["go"],
      filenames: [],
      mimetypes: ["text/x-go"],
      lsp: "gopls"
    },
    gomod: {
      color: 0x00add8,
      symbol: " ",
      extensions: ["mod"],
      filenames: [],
      mimetypes: ["text/x-go-mod"],
      lsp: "gopls"
    },
    haskell: {
      color: 0xa074c4,
      symbol: " ",
      extensions: ["hs", "lhs"],
      filenames: [],
      mimetypes: ["text/x-haskell"],
      lsp: "haskell-language-server"
    },
    html: {
      color: 0xef8a91,
      symbol: " ",
      extensions: ["html", "htm"],
      filenames: [],
      mimetypes: ["text/html"],
      lsp: "emmet-language-server"
    },
    javascript: {
      color: 0xf0df8a,
      symbol: " ",
      extensions: ["js"],
      filenames: [],
      mimetypes: ["application/javascript"],
      lsp: "typescript-language-server"
    },
    typescript: {
      color: 0x36a3d9,
      symbol: " ",
      extensions: ["ts"],
      filenames: [],
      mimetypes: ["application/typescript"],
      lsp: "typescript-language-server"
    },
    json: {
      color: 0xcbcb41,
      symbol: "{}",
      extensions: ["json"],
      filenames: [],
      mimetypes: ["application/json"],
      lsp: "vscode-json-language-server"
    },
    jsonc: {
      color: 0xcbcb41,
      symbol: "{}",
      extensions: ["jsonc"],
      filenames: [],
      mimetypes: ["application/json"],
      lsp: "vscode-json-language-server"
    },
    erb: {
      color: 0x6e1516,
      symbol: " ",
      extensions: ["erb"],
      filenames: [],
      mimetypes: ["text/x-erb"],
      lsp: "ruby-lsp"
    },
    lua: {
      color: 0x36a3d9,
      symbol: "󰢱 ",
      extensions: ["lua"],
      filenames: [],
      mimetypes: ["text/x-lua"],
      lsp: "lua-language-server"
    },
    python: {
      color: 0x95e6cb,
      symbol: "󰌠 ",
      extensions: ["py"],
      filenames: [],
      mimetypes: ["text/x-python"],
      lsp: "pyright"
    },
    rust: {
      color: 0xdea584,
      symbol: "󱘗 ",
      extensions: ["rs"],
      filenames: [],
      mimetypes: ["text/x-rust"],
      lsp: "rust-analyzer"
    },
    php: {
      color: 0xa074c4,
      symbol: "󰌟 ",
      extensions: ["php"],
      filenames: [],
      mimetypes: ["application/x-php"],
      lsp: "intelephense"
    },
    markdown: {
      color: 0x36a3d9,
      symbol: " ",
      extensions: ["md", "markdown"],
      filenames: [],
      mimetypes: ["text/markdown"],
      lsp: "marksman"
    },
    nginx: {
      color: 0x6d8086,
      symbol: " ",
      extensions: ["conf"],
      filenames: [],
      mimetypes: ["text/nginx"],
      lsp: "nginx-language-server"
    },
    toml: {
      color: 0x36a3d9,
      symbol: " ",
      extensions: ["toml"],
      filenames: [],
      mimetypes: ["application/toml"],
      lsp: "taplo"
    },
    yaml: {
      color: 0x6d8086,
      symbol: " ",
      extensions: ["yml", "yaml"],
      filenames: [],
      mimetypes: ["text/yaml"],
      lsp: "yaml-language-server"
    },
    sql: {
      color: 0xdad8d8,
      symbol: " ",
      extensions: ["sql"],
      filenames: [],
      mimetypes: ["text/x-sql"],
      lsp: "sqls"
    },
    make: {
      color: 0x4e5c61,
      symbol: " ",
      extensions: ["Makefile", "makefile"],
      filenames: [],
      mimetypes: ["text/x-makefile"],
      lsp: "make-language-server"
    },
    gdscript: {
      color: 0x6d8086,
      symbol: " ",
      extensions: ["gd"],
      filenames: [],
      mimetypes: ["text/x-gdscript"],
      lsp: nil
    },
    man: {
      color: 0xdad8d8,
      symbol: " ",
      extensions: ["man"],
      filenames: [],
      mimetypes: ["application/x-troff-man"],
      lsp: nil
    },
    diff: {
      color: 0xDD4C35,
      symbol: " ",
      extensions: ["diff", "patch"],
      filenames: [],
      mimetypes: ["text/x-diff"],
      lsp: nil
    },
    gitattributes: {
      color: 0xF05032,
      symbol: " ",
      extensions: ["gitattributes"],
      filenames: [],
      mimetypes: [],
      lsp: nil
    },
    gitignore: {
      color: 0xF05032,
      symbol: " ",
      extensions: ["gitignore"],
      filenames: [],
      mimetypes: [],
      lsp: nil
    },
    regex: {
      color: 0x9E9E9E,
      symbol: ".*",
      extensions: ["regex"],
      filenames: [],
      mimetypes: [],
      lsp: nil
    },
    ini: {
      color: 0x6d8086,
      symbol: " ",
      extensions: ["ini"],
      filenames: [],
      mimetypes: [],
      lsp: nil
    },
    ruby: {
      color: 0xff8087,
      symbol: "󰴭 ",
      extensions: ["rb"],
      filenames: ["Gemfile"],
      mimetypes: ["text/x-ruby"],
      lsp: "solargraph"
    },
    bash: {
      color: 0x4d5a5e,
      symbol: " ",
      extensions: ["sh"],
      filenames: ["bash_profile", "bashrc"],
      mimetypes: ["text/x-sh"],
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
    # rubocop:disable Lint/BooleanSymbol
    :true => { fg: 0x7AE93C },
    :false => { fg: 0xEF5168 },
    # rubocop:enable Lint/BooleanSymbol
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
  @log_queue = []
  @b_startup = nil
  @b_shutdown = nil

  class << self
    attr_accessor :theme, :lsp_config, :languages,
                  :line_endings, :highlighters
    attr_reader :b_startup, :b_shutdown, :b_extra_highlights

    def startup(&block)
      @b_startup = block
    end

    def shutdown(&block)
      @b_shutdown = block
    end

    def queue_log(msg)
      @log_queue << msg
    end

    def log_all
      @log_queue.each do |msg|
        puts msg
      end
      @log_queue = []
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

at_exit { C.log_all }
