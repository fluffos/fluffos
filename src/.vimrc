if &term=="vt100"
    set term=linux
endif

if &term=="vt100" || &term=="linux"
    set t_Co=8
    if has("terminfo")
        set t_Sf=[3%p1%dm
        set t_Sb=[4%p1%dm
    else
        set t_Sf=[3%dm
        set t_Sb=[4%dm
    endif
endif

if &term=="linux" || &term=="xterm"
    syntax on
endif

set ai
set nobackup
set bg=dark
set cin
set noexpandtab
set ru
set sw=4
set sc
set sm
set smarttab
set sts=4
set nowrap
set textwidth=0
set nohlsearch
set viminfo=""

hi PreProc cterm=bold ctermfg=2
hi Comment cterm=bold ctermfg=3
hi Constant cterm=bold ctermfg=6
hi Identifier ctermfg=7
hi Type ctermfg=7
hi Statement cterm=bold ctermfg=7
hi diffRemoved cterm=bold ctermfg=1
hi diffAdded cterm=bold ctermfg=3
hi diffChanged cterm=bold ctermfg=6
hi Search ctermfg=0 ctermbg=7
