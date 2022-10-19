NAME 	= Cub3d

INC 	= -I ./include -I ./include/inc
LIB		= -L ./include/lib -lft -lmlx_Linux
CC		= gcc
CFLAGS 	= -g3 -Wall -Werror -Wextra #-Wpedantic
SNTZ	= -fsanitize=address
RM		= rm -rf

FILES	= src/main \
		  src/hooks/hooks \
		  src/parser/parser \
		  src/render/render \
		  src/utils/utils \

SRC		= $(addsuffix .c, $(FILES))
OBJ		= $(addsuffix .o, $(FILES))

.c.o:
	$(CC) $(CFLAGS) $(SNTZ) $(INC) -o $@ -c $^

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(SNTZ) -o $(NAME) $(OBJ) $(LIB)

all: $(NAME)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

#run: all
#	./$(NAME)

.PHONY: all clean fclean re run