/* ************************************************************************** */
/*   test_libft.c - tests unitaires maison pour libft                         */
/*   Chaque groupe tourne dans un processus fils : un segfault est détecté    */
/*   et n'empêche pas les autres tests de s'exécuter.                         */
/* ************************************************************************** */

#include "libft.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <sys/wait.h>

/* ----------------------------- mini framework ---------------------------- */

static int g_failed;

#define SIGN(x) (((x) > 0) - ((x) < 0))

#define CHECK(cond) do { \
	if (!(cond)) { \
		g_failed++; \
		printf("    [KO] ligne %d : %s\n", __LINE__, #cond); \
	} \
} while (0)

/* compare une chaîne allouée avec l'attendue, puis la libère */
#define CHECK_STR(got, expected) do { \
	char *g_ = (got); \
	if (!g_ || strcmp(g_, (expected)) != 0) { \
		g_failed++; \
		printf("    [KO] ligne %d : attendu \"%s\", obtenu \"%s\"\n", \
			__LINE__, (expected), g_ ? g_ : "(NULL)"); \
	} \
	free(g_); \
} while (0)

/* capture ce qu'une fonction écrit sur `fd` */
#define CAPTURE(call, out) do { \
	int p_[2]; ssize_t r_; \
	if (pipe(p_) == -1) exit(2); \
	{ int fd = p_[1]; call; } \
	close(p_[1]); \
	r_ = read(p_[0], out, sizeof(out) - 1); \
	out[r_ < 0 ? 0 : r_] = '\0'; \
	close(p_[0]); \
} while (0)

/* --------------------- références pour les fonctions BSD ------------------ */

static size_t	ref_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	len = strlen(src);
	size_t	n;

	if (size)
	{
		n = (len >= size) ? size - 1 : len;
		memcpy(dst, src, n);
		dst[n] = '\0';
	}
	return (len);
}

static size_t	ref_strlcat(char *dst, const char *src, size_t size)
{
	size_t	dlen = 0;
	size_t	slen = strlen(src);
	size_t	n;

	while (dlen < size && dst[dlen])
		dlen++;
	if (dlen == size)
		return (size + slen);
	n = (slen >= size - dlen) ? size - dlen - 1 : slen;
	memcpy(dst + dlen, src, n);
	dst[dlen + n] = '\0';
	return (dlen + slen);
}

/* ------------------------------ Partie 1 --------------------------------- */

static void	test_is(void)
{
	int	c;

	for (c = -1; c <= 255; c++)
	{
		CHECK(!!ft_isalpha(c) == !!isalpha(c));
		CHECK(!!ft_isdigit(c) == !!isdigit(c));
		CHECK(!!ft_isalnum(c) == !!isalnum(c));
		CHECK(!!ft_isascii(c) == !!isascii(c));
		CHECK(!!ft_isprint(c) == !!isprint(c));
	}
}

static void	test_toupper_tolower(void)
{
	int	c;

	for (c = -1; c <= 255; c++)
	{
		CHECK(ft_toupper(c) == toupper(c));
		CHECK(ft_tolower(c) == tolower(c));
	}
}

static void	test_strlen(void)
{
	CHECK(ft_strlen("") == 0);
	CHECK(ft_strlen("a") == 1);
	CHECK(ft_strlen("hello world") == strlen("hello world"));
	CHECK(ft_strlen("abc\0def") == 3);
}

static void	test_memset(void)
{
	char	a[32];
	char	b[32];

	memset(a, 'X', sizeof(a));
	memset(b, 'X', sizeof(b));
	CHECK(ft_memset(a, 'A', 10) == a);
	memset(b, 'A', 10);
	CHECK(memcmp(a, b, sizeof(a)) == 0);
	ft_memset(a, 0, 0);
	CHECK(memcmp(a, b, sizeof(a)) == 0);
	ft_memset(a, 256 + 'Z', 5);		/* converti en unsigned char */
	memset(b, 256 + 'Z', 5);
	CHECK(memcmp(a, b, sizeof(a)) == 0);
}

static void	test_bzero(void)
{
	char	a[16];
	char	b[16];

	memset(a, 'X', sizeof(a));
	memset(b, 'X', sizeof(b));
	ft_bzero(a, 7);
	bzero(b, 7);
	CHECK(memcmp(a, b, sizeof(a)) == 0);
	ft_bzero(a, 0);
	CHECK(memcmp(a, b, sizeof(a)) == 0);
}

static void	test_memcpy(void)
{
	char	src[16] = "0123456789abcde";
	char	a[16];
	char	b[16];
	size_t	n;

	for (n = 0; n <= 16; n++)
	{
		memset(a, 'X', sizeof(a));
		memset(b, 'X', sizeof(b));
		CHECK(ft_memcpy(a, src, n) == a);
		memcpy(b, src, n);
		CHECK(memcmp(a, b, sizeof(a)) == 0);
	}
}

static void	test_memmove(void)
{
	char	a[32];
	char	b[32];

	/* chevauchement : dst > src (copie à rebours) */
	strcpy(a, "0123456789abcdefghij");
	strcpy(b, "0123456789abcdefghij");
	CHECK(ft_memmove(a + 3, a, 10) == a + 3);
	memmove(b + 3, b, 10);
	CHECK(memcmp(a, b, sizeof(a)) == 0);
	/* chevauchement : dst < src */
	strcpy(a, "0123456789abcdefghij");
	strcpy(b, "0123456789abcdefghij");
	ft_memmove(a, a + 3, 10);
	memmove(b, b + 3, 10);
	CHECK(memcmp(a, b, sizeof(a)) == 0);
	/* sans chevauchement / taille 0 */
	ft_memmove(a + 20, a, 5);
	memmove(b + 20, b, 5);
	ft_memmove(a, a + 1, 0);
	CHECK(memcmp(a, b, sizeof(a)) == 0);
}

static void	test_strlcpy(void)
{
	char	a[16];
	char	b[16];
	size_t	size;

	for (size = 0; size <= 14; size++)
	{
		memset(a, 'X', sizeof(a));
		memset(b, 'X', sizeof(b));
		CHECK(ft_strlcpy(a, "hello world", size)
			== ref_strlcpy(b, "hello world", size));
		CHECK(memcmp(a, b, sizeof(a)) == 0);
	}
	memset(a, 'X', sizeof(a));
	memset(b, 'X', sizeof(b));
	CHECK(ft_strlcpy(a, "", 5) == ref_strlcpy(b, "", 5));
	CHECK(memcmp(a, b, sizeof(a)) == 0);
}

static void	test_strlcat(void)
{
	char	a[16];
	char	b[16];
	size_t	size;

	for (size = 0; size <= 16; size++)
	{
		memset(a, 'X', sizeof(a));
		memset(b, 'X', sizeof(b));
		strcpy(a, "abc");
		strcpy(b, "abc");
		CHECK(ft_strlcat(a, "defgh", size) == ref_strlcat(b, "defgh", size));
		CHECK(memcmp(a, b, sizeof(a)) == 0);
	}
	/* dst vide */
	memset(a, 'X', sizeof(a));
	memset(b, 'X', sizeof(b));
	a[0] = '\0';
	b[0] = '\0';
	CHECK(ft_strlcat(a, "xyz", 8) == ref_strlcat(b, "xyz", 8));
	CHECK(memcmp(a, b, sizeof(a)) == 0);
}

static void	test_strchr_strrchr(void)
{
	const char	*s = "hello world";
	int			chars[] = {'h', 'o', 'l', 'd', 'z', '\0', 'o' + 256};
	size_t		i;

	for (i = 0; i < sizeof(chars) / sizeof(chars[0]); i++)
	{
		CHECK(ft_strchr(s, chars[i]) == strchr(s, chars[i]));
		CHECK(ft_strrchr(s, chars[i]) == strrchr(s, chars[i]));
	}
	CHECK(ft_strchr("", 'a') == strchr("", 'a'));
	CHECK(ft_strchr("", '\0') == strchr("", '\0'));
	CHECK(ft_strrchr("", '\0') == strrchr("", '\0'));
}

static void	test_strncmp(void)
{
	const char	*pairs[][2] = {
		{"", ""}, {"a", ""}, {"", "a"}, {"abc", "abc"}, {"abc", "abd"},
		{"abd", "abc"}, {"abc", "abcd"}, {"abcd", "abc"},
		{"test\200", "test\0"}, {"test\0", "test\200"}, {"zzz", "aaa"}};
	size_t		i;
	size_t		n;

	for (i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++)
		for (n = 0; n <= 6; n++)
			CHECK(SIGN(ft_strncmp(pairs[i][0], pairs[i][1], n))
				== SIGN(strncmp(pairs[i][0], pairs[i][1], n)));
}

static void	test_memchr(void)
{
	unsigned char	buf[] = {0, 1, 2, 3, 4, 5, 3, 7};
	int				c;
	size_t			n;

	for (c = 0; c <= 9; c++)
		for (n = 0; n <= sizeof(buf); n++)
			CHECK(ft_memchr(buf, c, n) == memchr(buf, c, n));
	CHECK(ft_memchr(buf, 256 + 3, sizeof(buf)) == memchr(buf, 256 + 3, sizeof(buf)));
	CHECK(ft_memchr(buf, 0, sizeof(buf)) == buf);
}

static void	test_memcmp(void)
{
	unsigned char	a[] = {1, 2, 3, 200, 5};
	unsigned char	b[] = {1, 2, 3, 100, 5};
	unsigned char	c[] = {1, 2, 3, 200, 5};
	size_t			n;

	for (n = 0; n <= 5; n++)
	{
		CHECK(SIGN(ft_memcmp(a, b, n)) == SIGN(memcmp(a, b, n)));
		CHECK(SIGN(ft_memcmp(b, a, n)) == SIGN(memcmp(b, a, n)));
		CHECK(SIGN(ft_memcmp(a, c, n)) == SIGN(memcmp(a, c, n)));
	}
}

static void	test_strnstr(void)
{
	const char	*h = "hello world";

	CHECK(ft_strnstr(h, "world", 11) == h + 6);
	CHECK(ft_strnstr(h, "world", 10) == NULL);
	CHECK(ft_strnstr(h, "hello", 5) == h);
	CHECK(ft_strnstr(h, "", 0) == h);
	CHECK(ft_strnstr(h, "", 5) == h);
	CHECK(ft_strnstr(h, "h", 0) == NULL);
	CHECK(ft_strnstr("", "a", 5) == NULL);
	CHECK(ft_strnstr("", "", 5) != NULL);
	CHECK(ft_strnstr("aaabc", "aab", 5) == "aaabc" + 1);
	CHECK(ft_strnstr("abc", "abcdef", 10) == NULL);
	CHECK(ft_strnstr(h, "xyz", 11) == NULL);
	CHECK(ft_strnstr(h, "o", 100) == h + 4);
}

static void	test_atoi(void)
{
	const char	*tests[] = {"42", "-42", "+42", "0", "-0", "   \t\n\v\f\r42abc",
		"", "abc", "--1", "+-1", "- 1", "  +  3", "2147483647", "-2147483648",
		"007", "12 34", "\n-5"};
	size_t		i;

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
	{
		if (ft_atoi(tests[i]) != atoi(tests[i]))
		{
			g_failed++;
			printf("    [KO] atoi(\"%s\") : attendu %d, obtenu %d\n",
				tests[i], atoi(tests[i]), ft_atoi(tests[i]));
		}
	}
}

static void	test_calloc_strdup(void)
{
	unsigned char	*p;
	size_t			i;
	int				ok;
	char			*d;

	p = ft_calloc(20, sizeof(int));
	CHECK(p != NULL);
	ok = 1;
	for (i = 0; p && i < 20 * sizeof(int); i++)
		if (p[i] != 0)
			ok = 0;
	CHECK(ok);
	free(p);
	p = ft_calloc(0, 0);
	free(p);					/* ne doit pas planter */
	CHECK(ft_calloc(SIZE_MAX, 2) == NULL);	/* overflow */
	d = ft_strdup("hello");
	CHECK(d != NULL && strcmp(d, "hello") == 0);
	free(d);
	d = ft_strdup("");
	CHECK(d != NULL && d[0] == '\0');
	free(d);
}

/* ------------------------------ Partie 2 --------------------------------- */

static void	test_substr(void)
{
	CHECK_STR(ft_substr("hello", 1, 3), "ell");
	CHECK_STR(ft_substr("hello", 0, 5), "hello");
	CHECK_STR(ft_substr("hello", 0, 100), "hello");
	CHECK_STR(ft_substr("hello", 3, 100), "lo");
	CHECK_STR(ft_substr("hello", 5, 3), "");
	CHECK_STR(ft_substr("hello", 100, 3), "");
	CHECK_STR(ft_substr("hello", 2, 0), "");
	CHECK_STR(ft_substr("", 0, 3), "");
}

static void	test_strjoin(void)
{
	CHECK_STR(ft_strjoin("hello", " world"), "hello world");
	CHECK_STR(ft_strjoin("", "abc"), "abc");
	CHECK_STR(ft_strjoin("abc", ""), "abc");
	CHECK_STR(ft_strjoin("", ""), "");
}

static void	test_strtrim(void)
{
	CHECK_STR(ft_strtrim("  hello  ", " "), "hello");
	CHECK_STR(ft_strtrim("xxhixyx", "xy"), "hi");
	CHECK_STR(ft_strtrim("hello", "xyz"), "hello");
	CHECK_STR(ft_strtrim("hello", ""), "hello");
	CHECK_STR(ft_strtrim("   ", " "), "");
	CHECK_STR(ft_strtrim("", " "), "");
	CHECK_STR(ft_strtrim("abcba", "ab"), "c");
	CHECK_STR(ft_strtrim("  a b  ", " "), "a b");
}

static void	check_split(const char *s, char c, const char **exp)
{
	char	**r = ft_split(s, c);
	size_t	n = 0;
	size_t	i;

	CHECK(r != NULL);
	if (!r)
		return ;
	while (exp[n])
		n++;
	for (i = 0; i < n; i++)
	{
		if (!r[i])
		{
			g_failed++;
			printf("    [KO] split(\"%s\") : trop peu d'éléments\n", s);
			break ;
		}
		if (strcmp(r[i], exp[i]) != 0)
		{
			g_failed++;
			printf("    [KO] split(\"%s\") [%zu] : attendu \"%s\", obtenu \"%s\"\n",
				s, i, exp[i], r[i]);
		}
	}
	for (i = 0; r[i]; i++)
		free(r[i]);
	if (i != n)
	{
		g_failed++;
		printf("    [KO] split(\"%s\") : %zu éléments au lieu de %zu\n", s, i, n);
	}
	free(r);
}

static void	test_split(void)
{
	const char	*e1[] = {"hello", "world", "foo", NULL};
	const char	*e2[] = {NULL};
	const char	*e3[] = {"abc", NULL};
	const char	*e4[] = {"a", "b", "c", NULL};

	check_split("hello world foo", ' ', e1);
	check_split("  hello   world  foo  ", ' ', e1);
	check_split("", ' ', e2);
	check_split("     ", ' ', e2);
	check_split("abc", ' ', e3);
	check_split("abc", 'x', e3);
	check_split("a,b,c", ',', e4);
	check_split(",,a,,b,,c,,", ',', e4);
	check_split("abc", '\0', e3);
}

static void	test_itoa(void)
{
	int		values[] = {0, 1, -1, 9, 10, -10, 42, -42, 12345, 2147483647,
		-2147483647, INT_MIN};
	char	expected[16];
	size_t	i;

	for (i = 0; i < sizeof(values) / sizeof(values[0]); i++)
	{
		snprintf(expected, sizeof(expected), "%d", values[i]);
		CHECK_STR(ft_itoa(values[i]), expected);
	}
}

static char	map_alt(unsigned int i, char c)
{
	return ((i % 2 == 0) ? (char)toupper(c) : c);
}

static void	iter_alt(unsigned int i, char *c)
{
	if (i % 2 == 0)
		*c = (char)toupper(*c);
}

static void	test_strmapi_striteri(void)
{
	char	buf[] = "abcdef";

	CHECK_STR(ft_strmapi("abcdef", map_alt), "AbCdEf");
	CHECK_STR(ft_strmapi("", map_alt), "");
	ft_striteri(buf, iter_alt);
	CHECK(strcmp(buf, "AbCdEf") == 0);
	buf[0] = '\0';
	ft_striteri(buf, iter_alt);
	CHECK(buf[0] == '\0');
}

static void	test_put_fd(void)
{
	char	out[64];
	int		values[] = {0, 7, -7, 42, -42, 1000, INT_MAX, INT_MIN};
	char	expected[16];
	size_t	i;

	CAPTURE(ft_putchar_fd('A', fd), out);
	CHECK(strcmp(out, "A") == 0);
	CAPTURE(ft_putstr_fd("hello", fd), out);
	CHECK(strcmp(out, "hello") == 0);
	CAPTURE(ft_putstr_fd("", fd), out);
	CHECK(strcmp(out, "") == 0);
	CAPTURE(ft_putendl_fd("hello", fd), out);
	CHECK(strcmp(out, "hello\n") == 0);
	for (i = 0; i < sizeof(values) / sizeof(values[0]); i++)
	{
		snprintf(expected, sizeof(expected), "%d", values[i]);
		CAPTURE(ft_putnbr_fd(values[i], fd), out);
		CHECK(strcmp(out, expected) == 0);
	}
}

/* ------------------------------- Bonus ----------------------------------- */

static void	del_free(void *p)
{
	free(p);
}

static int	g_iter;

static void	iter_count(void *c)
{
	char	*s = c;

	g_iter++;
	s[0] = (char)toupper(s[0]);
}

static void	*map_dup(void *c)
{
	char	*s = strdup(c);

	s[0] = (char)toupper(s[0]);
	return (s);
}

static t_list	*mk(const char *s)
{
	return (ft_lstnew(strdup(s)));
}

static void	test_lst_basic(void)
{
	int		a = 1, b = 2, c = 3;
	t_list	*l = NULL;
	t_list	*n;

	n = ft_lstnew(&a);
	CHECK(n != NULL && n->content == &a && n->next == NULL);
	CHECK(ft_lstsize(NULL) == 0);
	CHECK(ft_lstlast(NULL) == NULL);
	ft_lstadd_front(&l, n);					/* [a] */
	CHECK(l == n && ft_lstsize(l) == 1);
	ft_lstadd_front(&l, ft_lstnew(&b));		/* [b, a] */
	CHECK(l->content == &b && l->next == n && ft_lstsize(l) == 2);
	ft_lstadd_back(&l, ft_lstnew(&c));		/* [b, a, c] */
	CHECK(ft_lstsize(l) == 3);
	CHECK(ft_lstlast(l)->content == &c);
	CHECK(ft_lstlast(l)->next == NULL);
	while (l)
	{
		n = l->next;
		free(l);
		l = n;
	}
}

static void	test_lst_add_back_empty(void)
{
	int		a = 1;
	t_list	*l = NULL;
	t_list	*n = ft_lstnew(&a);

	ft_lstadd_back(&l, n);
	CHECK(l == n);
	CHECK(ft_lstsize(l) == 1);
	free(l);
}

static void	test_lst_delone_clear_iter(void)
{
	t_list	*l = NULL;

	ft_lstdelone(mk("solo"), del_free);		/* ne doit pas fuiter */
	ft_lstadd_back(&l, mk("abc"));
	ft_lstadd_back(&l, mk("def"));
	ft_lstadd_back(&l, mk("ghi"));
	g_iter = 0;
	ft_lstiter(l, iter_count);
	CHECK(g_iter == 3);
	CHECK(strcmp(l->content, "Abc") == 0);
	CHECK(strcmp(l->next->content, "Def") == 0);
	CHECK(strcmp(ft_lstlast(l)->content, "Ghi") == 0);
	ft_lstclear(&l, del_free);
	CHECK(l == NULL);
	ft_lstclear(&l, del_free);				/* liste vide : pas de crash */
	ft_lstiter(NULL, iter_count);
}

static void	test_lst_map(void)
{
	t_list	*l = NULL;
	t_list	*m;

	ft_lstadd_back(&l, mk("abc"));
	ft_lstadd_back(&l, mk("def"));
	ft_lstadd_back(&l, mk("ghi"));
	m = ft_lstmap(l, map_dup, del_free);
	CHECK(m != NULL && ft_lstsize(m) == 3);
	CHECK(m != l);
	if (m && ft_lstsize(m) == 3)
	{
		CHECK(strcmp(m->content, "Abc") == 0);
		CHECK(strcmp(m->next->content, "Def") == 0);
		CHECK(strcmp(m->next->next->content, "Ghi") == 0);
	}
	CHECK(strcmp(l->content, "abc") == 0);	/* original intact */
	ft_lstclear(&m, del_free);
	ft_lstclear(&l, del_free);
}

/* -------------------------------- main ----------------------------------- */

typedef struct s_test
{
	const char	*name;
	void		(*fn)(void);
}	t_test;

int	main(int argc, char **argv)
{
	static const t_test	tests[] = {
		{"is* (alpha/digit/alnum/ascii/print)", test_is},
		{"toupper / tolower", test_toupper_tolower},
		{"strlen", test_strlen}, {"memset", test_memset},
		{"bzero", test_bzero}, {"memcpy", test_memcpy},
		{"memmove", test_memmove}, {"strlcpy", test_strlcpy},
		{"strlcat", test_strlcat}, {"strchr / strrchr", test_strchr_strrchr},
		{"strncmp", test_strncmp}, {"memchr", test_memchr},
		{"memcmp", test_memcmp}, {"strnstr", test_strnstr},
		{"atoi", test_atoi}, {"calloc / strdup", test_calloc_strdup},
		{"substr", test_substr}, {"strjoin", test_strjoin},
		{"strtrim", test_strtrim}, {"split", test_split},
		{"itoa", test_itoa}, {"strmapi / striteri", test_strmapi_striteri},
		{"put*_fd", test_put_fd},
		{"lst: new/add_front/add_back/size/last", test_lst_basic},
		{"lst: add_back sur liste vide", test_lst_add_back_empty},
		{"lst: delone/clear/iter", test_lst_delone_clear_iter},
		{"lst: map", test_lst_map},
	};
	size_t	i;
	int		status;
	int		ko = 0;
	size_t	count = sizeof(tests) / sizeof(tests[0]);

	for (i = 0; i < count; i++)
	{
		if (argc > 1 && !strstr(tests[i].name, argv[1]))
			continue ;			/* filtre : ./run_tests split */
		printf("%-42s\n", tests[i].name);
		fflush(stdout);
		if (fork() == 0)
		{
			tests[i].fn();
			exit(g_failed ? 1 : 0);
		}
		wait(&status);
		if (WIFSIGNALED(status))
		{
			printf("    [CRASH] signal %d (%s)\n", WTERMSIG(status),
				strsignal(WTERMSIG(status)));
			ko++;
		}
		else if (WEXITSTATUS(status) != 0)
			ko++;
		else
			printf("    [OK]\n");
	}
	printf("\n%s : %d groupe(s) en échec\n", ko ? "RÉSULTAT" : "TOUT EST OK", ko);
	return (ko != 0);
}
