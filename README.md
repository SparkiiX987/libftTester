# libft_tests

Suite de tests unitaires en C pour la [libft](https://github.com/) (projet 42), sans dépendance externe.

- Les fonctions qui existent dans la libc sont comparées directement à celle-ci.
- Chaque groupe de tests tourne dans un processus séparé (`fork`) : un segfault est signalé par `[CRASH]` et n'interrompt pas les autres tests.
- Compilation avec **AddressSanitizer** et **UBSan** pour détecter fuites mémoire et accès invalides.

## Fonctions testées

| Catégorie | Fonctions |
|---|---|
| Libc | `ft_isalpha`, `ft_isdigit`, `ft_isalnum`, `ft_isascii`, `ft_isprint`, `ft_strlen`, `ft_memset`, `ft_bzero`, `ft_memcpy`, `ft_memmove`, `ft_toupper`, `ft_tolower`, `ft_strchr`, `ft_strrchr`, `ft_strncmp`, `ft_memchr`, `ft_memcmp`, `ft_atoi`, `ft_calloc`, `ft_strdup` |
| BSD (référence intégrée) | `ft_strlcpy`, `ft_strlcat`, `ft_strnstr` |
| Partie 2 | `ft_substr`, `ft_strjoin`, `ft_strtrim`, `ft_split`, `ft_itoa`, `ft_strmapi`, `ft_striteri`, `ft_putchar_fd`, `ft_putstr_fd`, `ft_putendl_fd`, `ft_putnbr_fd` |
| Bonus (listes) | `ft_lstnew`, `ft_lstadd_front`, `ft_lstsize`, `ft_lstlast`, `ft_lstadd_back`, `ft_lstdelone`, `ft_lstclear`, `ft_lstiter`, `ft_lstmap` |

## Installation

Place ce dépôt dans un sous-dossier de ta libft, de façon à ce que `libft.h` et le `Makefile` de la libft soient dans le dossier parent :

```
libft/
├── Makefile
├── libft.h
├── ft_*.c
└── libft_tests/        <- ce dépôt
    ├── Makefile
    ├── README.md
    └── test_libft.c
```

Par exemple :

```bash
cd libft
git clone <url-de-ce-depot> libft_tests
```

Si ta libft est ailleurs, modifie la variable `LIBFT_DIR` dans le `Makefile`.

## Utilisation

```bash
cd libft_tests
make test              # compile la libft + les tests et lance tout
```

Lancer uniquement certains groupes (filtre sur le nom) :

```bash
./run_tests split
./run_tests lst
```

Nettoyer :

```bash
make clean
```

> **Prérequis :** la libft doit produire une bibliothèque `libft.a` avec `make` (règle `all` ou par défaut) et être compilable avec `cc`.

## Exemple de sortie

```
strlen
    [OK]
split
    [KO] ligne 312 : r != NULL
lst: map
    [CRASH] signal 11 (Segmentation fault)

RÉSULTAT : 2 groupe(s) en échec
```

- `[OK]` : tous les tests du groupe passent.
- `[KO]` : une assertion a échoué (la ligne et la condition sont affichées).
- `[CRASH]` : le groupe a été tué par un signal (segfault, abort…). Une erreur détectée par ASan (fuite, overflow) fait aussi échouer le groupe.

## Ajouter un test

1. Écris une fonction dans `test_libft.c` :

   ```c
   static void test_ma_fonction(void)
   {
       CHECK(ft_strlen("abc") == 3);
       CHECK_STR(ft_strjoin("a", "b"), "ab");   /* compare puis free */
   }
   ```

2. Ajoute-la au tableau `tests[]` dans `main` :

   ```c
   {"ma fonction", test_ma_fonction},
   ```

Macros disponibles :

| Macro | Rôle |
|---|---|
| `CHECK(cond)` | Échoue si `cond` est fausse |
| `CHECK_STR(got, expected)` | Compare une chaîne allouée à l'attendue, puis la libère |
| `CAPTURE(call, out)` | Capture dans `out` ce qu'une fonction `*_fd` écrit sur `fd` |
| `SIGN(x)` | Signe d'un entier, utile pour comparer `strcmp`/`memcmp` |

## Limites

- Les comportements indéfinis (par exemple `ft_strlen(NULL)`) ne sont volontairement pas testés, puisque la libc elle-même plante dans ces cas.
- Les échecs de `malloc` ne sont pas simulés.
- Ces tests ne remplacent pas la moulinette ni la relecture par les pairs ; ils servent à détecter les erreurs courantes avant la correction.

## Licence

Libre d'utilisation. Adapte-les comme tu veux.