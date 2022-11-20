#include "cub3d.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>


/**
 * Abrimos el fichero y leemos todo la informacion
 * @param path ruta del fichero
 * @param sz numero de filas del fichero
 * @return NULL si sale mal, el mapa en un char ** si sale bien
 */
char **load_raw_file_data(char *path, size_t sz)
{
	char	**map;
	char	*line;
	int		fd;

	map = ft_calloc(sizeof(char *), sz + 1);
	if (!map)
		return (NULL);
	sz = 0;
	fd = open(path, O_RDONLY);
	while (1)
	{
		line = get_next_line(fd);
		if (!line)
			break ;
        //GRACIAS NORMINETTE
        if (line[0] == NL)
        {
            free(line);
            continue ;
        }
		map[sz++] = line;
	}
	close(fd);
	return (map);
}

static char **process_map(char **raw, t_player *player)
{
    char    **tmp;
    int     size;
    int     line;
    int     column;
    int     pos_found;

    pos_found = 0;
    size = str_array_get_size(raw);
    if (size == 0)
        return (NULL);
    tmp = malloc(sizeof(char*) * size);
    if (!tmp)
        return (NULL);
    line = 0;
    while (raw[line])
    {
        column = 0;
        while (raw[line][column] != '\0')
        {
            if (is_player_position(raw[line][column]) == 0)
            {
                if (process_player(pos_found, line, column, raw) < 0)
                    return (NULL);
                player->pos_y = line;
                player->dir_x = column;
                pos_found++;
            }
            column++;
        }
        tmp[line] = ft_strdup(raw[line]);
        line++;
    }
    if (pos_found > 1)
        return (NULL);
    return (tmp);
}

static void set_texture_keys(t_tx tx[4], t_rgb rgb[2])
{
    ft_memcpy(tx[0].key, NORTH, 2);
    ft_memcpy(tx[1].key, SOUTH, 2);
    ft_memcpy(tx[2].key, WEST, 2);
    ft_memcpy(tx[3].key, EAST, 2);
    ft_memcpy(rgb[0].key, "F", 1);
    ft_memcpy(rgb[1].key, "C", 1);
}

static int  set_rgb(t_rgb rgb[2], char **rgb_tab, int pos)
{
    size_t  i;
    int     num;
    char    **tab;

    tab = ft_split(rgb_tab[1], COMA);
    i = -1;
    while (tab[++i])
    {
        num = ft_atoi(tab[i]);
        if (num < 0 || num > 255)
        {
            free_str_array(tab);
            return (-1);
        }
        rgb[pos].trgb[i + 1] = num;
    }
    return (0);
}

static int query_texture(t_mlx *mlx, t_tx tx[4], t_rgb rgb[2], char **tx_tab)
{
    size_t  i;

    i = -1;
    while (++i < 4)
    {
        if (!ft_strncmp(tx[i].key, tx_tab[0], 3))
        {
            if (process_image(&tx[i], mlx, tx_tab[1]))
                return (-1);
            break ;
            }
        else if (!ft_strncmp(rgb[0].key, tx_tab[0], 2))
        {
            set_rgb(rgb, tx_tab, F);
            break ;
        }
        else if (!ft_strncmp(rgb[1].key, tx_tab[0], 2))
        {
            set_rgb(rgb, tx_tab, C);
            break ;
        }
    }
    return (0);
}

static int set_textures(t_tx tx[4], t_mlx *mlx, t_rgb rgb[2], char **raw_tab)
{
    char    **tx_tab;
    int     pos;
    int     config_cnt;

    set_texture_keys(tx, rgb);
    config_cnt = 0;
    pos = -1;
    while (raw_tab[++pos] && config_cnt < 6)
    {
        tx_tab = ft_split(raw_tab[pos], SP);
        if (!tx_tab || !tx_tab[0] || !tx_tab[1])
        {
            free_str_array(tx_tab);
            return (-1);
        }
        if (query_texture(mlx, tx, rgb, tx_tab) == -1)
            return (-1);
        config_cnt++;
        free_str_array(tx_tab);
    }
    if (config_cnt != 6)
        return (-1);
    return (pos);
}

static int  is_open(char **map, int i)
{
    int sum[2];
    int col;
    
    col = 1;
    while (map[i][col] && map[i + 1][col])
    {
        sum[0] = map[i + 1][col] + map[i][col];
        sum[1] = map[i][col + 1] + map[i][col];
        if (sum[0] == 80 || sum[1] == 80 || sum[0] == 48 || sum[1] == 48)
            return (1);
        col++;
    }
    if (map[i + 1][col] == '0' || map[i][col] == '0')
        return (1);
    return (0);
}

static char **get_map(char **raw, int start, size_t sz[2])
{
    char    **map;
    int     i;
    int     end;

    end = sz[0] - start - 1;
    map = ft_calloc(sizeof(char *), (sz[0] - start + 3));
    map[0] = ft_calloc(sizeof(char), (sz[1] + 1));
    ft_memset(map[0], SP, sz[1]);
    map[end] = ft_calloc(sizeof(char), (sz[1] + 1));
    ft_memset(map[end], SP, sz[1]);
    i = 0;
    while (raw[start])
    {
        map[i + 1] = raw[start++];
        if (is_open(map, i))
            return (NULL);
        i++;
    }
    return (map);
}

static char **complex_map(t_application *appl, char **raw_tab, size_t sz[2])
{
    int     pos;
    char    **map;

    pos = set_textures(appl->tx, &appl->mlx_win, appl->rgb, raw_tab);
    map = get_map(raw_tab, pos, sz);
    show_map(map);
    fprintf(stderr, "POS: %d\n", pos);
    return (NULL);
}

/**
 * First we are going to check if it is a simple map
 * or a complex one. If the first row with data is formed
 * by 1 or 'sp' we know that the map is simple, otherwise
 * map is complex
 * @param texture struct with data about textures and colors
 * @param raw map raw data
 * @return
 */
static char **process_raw_data(t_application *appl, char **raw, size_t sz[2])
{
    int     map_type;
    char    **map;

    map_type = get_map_type(raw);
    if (map_type == 0)
    {
        map = process_map(raw, &(appl->player));
        printf("Mapa de tipo simple\n");
        return (map);
    }
    else if (map_type == 1)
    {
        complex_map(appl, raw, sz);
        printf("Mapa de tipo complejo\n");

        return (NULL);
        //MAPA COMPLEJO
    }
    else if (map_type == -1)
        return (NULL);
    appl = NULL;
    return (NULL);
}

/**
 * We take all the data on the file to start the
 * parse process
 * @param appl struct with all the data
 * @param path file path string
 * @return NULL if file is not correct or a char**
 *          with the map
 */
char    **load_map(t_application *appl, char *path)
{
	size_t	sz[2];
    char    **tmp;
    char    **map;

    get_map_size(path, sz);
    if (!sz[0])
        return (NULL);
    tmp = load_raw_file_data(path, sz[0]);
    map = process_raw_data(appl, tmp, sz);
    free_str_array(tmp);
    if (!map)
        return (NULL);
    return (map);
}

/**
 * We get the map path and we try to make first checks:
 *      - File format needs to be .cub
 *      - File have to exist
 *      - We need to have permission
 *        to open and read
 * @param path la ruta del mapa
 * @return -1 if any error, 0 if it is correct
 */
int check_path_format(char *path, char *term)
{
    char    **tmp;
    int     size;
    int     fd;

    size = 0;
    tmp = ft_split(path, '.');
    if (!tmp)
        return (-1);
    fprintf(stderr, "COMNPLETE PATH: %s\n", path);
    while (tmp[size] != NULL)
        size++;
    for (int i = 0; tmp[i]; i++)
        fprintf(stderr, "tmp[%d] -> %s\n", i, tmp[i]);
    fd = open(path, O_RDONLY);
    close(fd);
    //GRACIASS NORMINETTE
    if (size <= 1 ||
        ft_strncmp(term, tmp[size - 1], ft_strlen(term) + 1) != 0 ||
        fd <= 0)
    {
        free_str_array(tmp);
        return (-1);
    }
    //free_str_array
    return (0);
}
