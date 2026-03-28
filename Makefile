# ===================================================================
# FLAGS
# ===================================================================
flags = -D_DEBUG -Werror -ggdb3 -lSDL2 -std=c++17 -O3 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

sdl2 =  -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer


# ===================================================================
# FILES
# ===================================================================
graphic = Graphic/GK_GraphicFunc.cpp
parser 	= Parser/GK_ParserFunc.cpp
tree 	= Tree/GK_TreeFunc.cpp

main 	= main.cpp

# -------------------------------------------------------------------
debug_files 	= $(graphic) $(parser) $(tree) $(main)


# ===================================================================
# DIRECTORY
# ===================================================================
graphic_dir = Graphic
parser_dir	= Parser
tree_dir	= Tree
music_dir	= Music
source_dir 	= Source

# -------------------------------------------------------------------
debug_dir_flags	= -I$(graphic_dir) -I$(parser_dir) -I$(tree_dir) -I$(source_dir) -I$(music_dir)

# ===================================================================
# BUILDS
# ===================================================================
debug:
	g++ $(debug_files) $(flags) $(debug_dir_flags) $(sdl2)  -o  gk



run:
	./gk
