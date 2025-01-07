" Basic Neovim configuration

" Enable line numbers (absolute)
set number

" Disable relative line numbers (so they don’t change dynamically as you scroll)
" Remove the following line to keep relative line numbers
" set relativenumber

" Disable cursor line highlighting
set nocursorline

" Enable syntax highlighting
syntax enable

" Enable filetype detection (important for proper C recognition)
filetype plugin indent on

" Set the background color (dark mode)
set background=dark

" Enable auto-indentation
set smartindent
set tabstop=4
set shiftwidth=4
set expandtab
set autoindent
set smarttab
set cindent

" Use spaces instead of tabs
set expandtab

" Set the indent size for C code specifically
autocmd FileType c setlocal tabstop=4 shiftwidth=4 softtabstop=4 expandtab

" Enable line highlighting for the current line
" set cursorline      " Remove this line to stop highlighting the current line

" Set search highlighting
set hlsearch
set incsearch

" Enable clipboard support (for system clipboard)
set clipboard=unnamedplus

" Set a custom color scheme (optional)
colorscheme desert

" Enable mouse support
set mouse=a

" Set the default search to ignore case
set ignorecase

" Make search case-sensitive if capital letter is used
set smartcase

" Show line and column number in status line
set ruler

" Show matching parentheses
set showmatch

" Auto-save files when switching buffers
set autowrite

" Enable automatic C code formatting with clang-format (if installed)
au BufWritePre *.c,*.cpp,*.h,*.hpp execute ':silent !clang-format -i %'

" Automatically reload C files if they are modified externally
au BufReadPost *.c,*.cpp,*.h,*.hpp setlocal autoread

