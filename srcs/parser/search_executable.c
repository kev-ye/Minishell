/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   search_executable.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaye <kaye@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/20 17:51:02 by besellem          #+#    #+#             */
/*   Updated: 2021/05/22 16:02:06 by kaye             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*find_exec(char **exectbl, char *command)
{
	char	*cmd;
	int		fd;
	size_t	i;

	i = 0;
	while (exectbl[i])
	{
		ft_asprintf(&cmd, "%s/%s", exectbl[i++], command);
		if (!cmd)	// may want to exit() minishell when this happens
			continue ;
		fd = open(cmd, O_RDONLY);
		if (fd != -1)
		{
			close(fd);
			return (cmd);
		}
		///////////////////////////////kaye
		//printf("cmd : %s\n", cmd); /// dont underst why can open if file no exist
		///////////////////////////////////
		ft_memdel((void **)&cmd);
	}
	return (NULL);
}

char	*search_executable(char *command)
{
	const char	*path = getenv("PATH");
	char		**exectbl;
	char		*cmd;

	if (!path)
	{
		///////////////// kaye
		printf("Path are unset ... maybe here need add some check function\n"); // need add check function for no found message
		//////////////////////
		return (NULL);
	}
	exectbl = ft_split(path, ':');
	if (!exectbl)
		return (NULL);
	cmd = find_exec(exectbl, command);
	ft_strsfree(ft_strslen(exectbl), exectbl);
	return (cmd);
}

char	*search_builtin_executable(char *command)
{
	const t_builtin builtin[BUILTIN] = {{"echo", ft_echo, NULL}, 
				{"cd", ft_cd, NULL}, {"pwd", NULL, ft_pwd}, 
				{"env", ft_env, NULL}, {"unset", ft_unset, NULL}, 
				{"export", NULL, NULL}, {"exit", NULL, ft_exit}};
	int i;

	i = 0;
	while (i < BUILTIN)
	{
		if (!ft_strcmp(command, builtin[i].cmd))
			return (command);
		++i;
	}
	return (NULL);
}