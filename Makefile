NAME		= run_tests
LIBFT_DIR	= ..
CC			= cc
CFLAGS		= -Wall -Wextra -g -fsanitize=address,undefined -I$(LIBFT_DIR)

all: $(NAME)

$(NAME): test_libft.c
	$(MAKE) -C $(LIBFT_DIR)
	$(CC) $(CFLAGS) test_libft.c -L$(LIBFT_DIR) -lft -o $(NAME)

test: $(NAME)
	ASAN_OPTIONS=allocator_may_return_null=1 ./$(NAME)

clean:
	rm -f $(NAME)

.PHONY: all test clean
