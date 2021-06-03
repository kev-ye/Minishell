/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaye <kaye@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/20 22:33:29 by besellem          #+#    #+#             */
/*   Updated: 2021/06/03 19:20:36 by kaye             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// seems that it must be used on forked commands - not on the main program
void	ft_quit(int code)
{
	ft_dprintf(STDERR_FILENO, B_RED "SIGNAL: Quit: %d\n" CLR_COLOR, code);
	// exit(code);
}

void	ft_interrupt(int code)
{
	// kill(singleton()->thread_pid, code);	
	// exit(code);
	(void)code;
	ft_putstr_fd("\n", STDERR_FILENO);
	print_prompt();
}

int ft_exec_builtin_cmd(char **cmds)
{
	static t_builtin	builtin[] = {
		{"echo", ft_echo, NULL}, {"cd", ft_cd, NULL}, {"pwd", NULL, ft_pwd},
		{"env", ft_env, NULL}, {"unset", ft_unset, NULL}, 
		{"export", ft_export, NULL}, {"exit", NULL, ft_exit},
		{"clear", NULL, ft_clear}, {NULL, NULL, NULL}
	};
	int					i;

	i = 0;
	while (builtin[i].cmd)
	{
		if (!ft_strcmp(cmds[0], builtin[i].cmd))
		{
			if (!builtin[i].f1)
				return (!builtin[i].f2());
			else
				return (!builtin[i].f1(cmds));
		}
		++i;
	}
	return (NOT_FOUND);
}

int	ft_exec_cmd(char *file, t_cmd *cmds)
{
	char	**env;
	int		ret = 0;

	env = ft_lst2strs(&singleton()->env);
	singleton()->last_return_value = execve(file, cmds->args, env);
	ft_memdel((void **)env);	// ne passe jamais ici
	return (ret);
	// if (-1 == ret)
	// 	return (ERROR);
	// else
	// 	return (ret);
}

// void	ft_pre_exec_cmd(void *ptr)
// {
// 	t_cmd	*cmd;
// 	char	*ex;
// 	int bl;

// 	cmd = ptr;
// 	if (!cmd->args || !*cmd->args)
// 		return ;
// 	ex = search_executable(cmd->args[0]);
// 	bl = ft_exec_builtin_cmd(char **cmds)
// 	if (bl)
// 	{
// 		ft_printf(B_RED "`%s' builtin command:\n" CLR_COLOR, bl);
// 		singleton()->last_return_value = ft_exec_builtin_cmd(cmd->args);
// 	}
// 	else if (ex)
// 	{
// 		ft_printf(B_RED "`%s' command:\n" CLR_COLOR, ex);
// 		singleton()->last_return_value = ft_exec_cmd(ex, cmd);
// 		ft_memdel((void **)&ex);
// 	}
// 	ft_strsfree(ft_strslen(cmd->args) + 1, cmd->args);
// }

void	ft_pre_exec_cmd(void *ptr)
{
	t_cmd	*cmd;
	char	*ex;

    cmd = ptr;
    if (!cmd->args || !cmd->args)
        return ;
    singleton()->last_return_value = ft_exec_builtin_cmd(cmd->args);
    if (singleton()->last_return_value == NOT_FOUND)
    {
		// printf("\n\n\n\n\n\nim here\n\n\n\n\n\n");
        ex = search_executable(cmd->args[0]);
        if (ex)
        {
            // ft_printf(B_RED "`%s' command:\n" CLR_COLOR, ex);
            singleton()->last_return_value = ft_exec_cmd(ex, cmd);
            ft_memdel((void **)&ex);
        }
        else
        {
            ft_dprintf(STDERR_FILENO, PROG_NAME ": %s: command not found\n",
                cmd->args[0]);
        }
    }
    ft_strsfree(ft_strslen(cmd->args) + 1, cmd->args);
}

/////////////////////////////////////////////////////////////////////////////////////////////// start pipe

int	*first_cmd_with_pipe(void *cmd)
{
	pid_t	pid;
	int		*fd = malloc(sizeof(int) * 2);

	if (!fd)
		return (NULL);
	pipe(fd);
	pid = fork();
	if (pid < 0)
			exit(1);
	else if (pid == 0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		ft_pre_exec_cmd(cmd);
		close(fd[1]);
		exit(0);
	}
	else
	{
		close(fd[1]);
		wait(NULL);
	}
	return (fd);
}

int	*interm_cmd_with_pipe(void *cmd, int *get_fd)
{
	pid_t	pid;
	int		*tmp = get_fd;
	int		*fd = malloc(sizeof(int) * 2);

	if (!fd)
		return (NULL);
	pipe(fd);
	pid = fork();
	if (pid < 0)
		exit(ERROR);
	else if (pid == 0)
	{
		close(get_fd[1]);
		dup2(get_fd[0], STDIN_FILENO);
		close(get_fd[0]);
	
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		ft_pre_exec_cmd(cmd);
		close(fd[1]);
		exit(0);
	}
	else
	{
		close(fd[1]);
		wait(NULL);
	}
	free(tmp);
	return (fd);
}

void	last_cmd_with_pipe(void *cmd, int *get_fd)
{
	const pid_t	pid = fork();
	int		*tmp = get_fd;

	if (pid < 0)
		exit(ERROR);
	else if (0 == pid)
	{
		close(get_fd[1]);
		dup2(get_fd[0], STDIN_FILENO);
		ft_pre_exec_cmd(cmd);
		close(get_fd[0]);
		exit(0);
	}
	else
	{
		close(get_fd[1]);
		wait(NULL);
	}
	free(tmp);
}

void	simple_cmd(void *cmd)
{
	pid_t	pid;

	if (!ft_strcmp(((t_cmd *)cmd)->args[0], "cd"))
		ft_pre_exec_cmd(cmd);
	else
	{	
		pid = fork();
		if (pid < 0)
				exit(1);
		else if (pid == 0)
		{
			ft_pre_exec_cmd(cmd);
			exit(0);
		}
		else
			wait(NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////// end pipe

/////////////////////////////////////////////////////////////////////////////////////////////// start direction

void *get_complete_cmd(void *cmd, t_list *lst_cmd)
{
	t_list	*lst_tmp;
	size_t len;
	char **new_cmd;
	int i;
	int j;

	lst_tmp = lst_cmd;
	while (lst_tmp && (((t_cmd *)lst_tmp->content)->status_flag & FLG_OUTPUT || ((t_cmd *)lst_tmp->content)->status_flag & FLG_APPEND || ((t_cmd *)lst_tmp->content)->status_flag & FLG_INPUT))
		lst_tmp = lst_tmp->next;
	len = ft_strslen(((t_cmd *)lst_tmp->content)->args);
	if (lst_tmp && len > 1)
	{
		len += ft_strslen(((t_cmd *)cmd)->args);
		new_cmd = malloc(sizeof(char *) * (len + 1));
		if (!new_cmd)
			return (NULL);
		i = 0;
		while (((t_cmd *)cmd)->args[i])
		{
			new_cmd[i] = ft_strdup(((t_cmd *)cmd)->args[i]);
			++i;
			--len;
		}
		j = 1;
		while (((t_cmd *)lst_tmp->content)->args[j] && len)
		{
			new_cmd[i] = ft_strdup(((t_cmd *)lst_tmp->content)->args[j]);
			++i;
			++j;
			--len;
		}
		new_cmd[i] = NULL;
		ft_strsfree(ft_strslen(((t_cmd *)cmd)->args), ((t_cmd *)cmd)->args);
		((t_cmd *)cmd)->args = new_cmd;
	}
	return (cmd);
}

int	redir_cmd(t_list *lst_cmd)
{
	int tmp_fd;
	int flag_append;
	int flag_trunc;
	int flag_redir_input;
	int first;

	tmp_fd = -1;
	flag_append = 0;
	flag_trunc = 0;
	first = 1;
	flag_redir_input = 0;
	while (lst_cmd)
	{
		// printf("in output\n");
		if (first == 1)
		{
			first = 0;
			if (((t_cmd *)lst_cmd->content)->status_flag & FLG_INPUT)
				flag_redir_input = 1;
			else if (((t_cmd *)lst_cmd->content)->status_flag & FLG_OUTPUT)
				flag_trunc = 1;
			else if (((t_cmd *)lst_cmd->content)->status_flag & FLG_APPEND)
				flag_append = 1;	
		}
		else if (!first && (flag_trunc == 1 || flag_append == 1))
		{
			if (flag_trunc == 1)
				tmp_fd = open(((t_cmd *)lst_cmd->content)->args[0], O_WRONLY | O_TRUNC | O_CREAT, 0666);                                       /////////// last time here -> redir_cmd remake  ////////// check with cat > f1 < f2
			else if (flag_append == 1)
				tmp_fd = open(((t_cmd *)lst_cmd->content)->args[0], O_WRONLY | O_APPEND | O_CREAT, 0666);
			if (tmp_fd == -1)
			{
				printf("check\n");
				ft_dprintf(STDERR_FILENO, "minishell: %s: %s\n", ((t_cmd *)lst_cmd->content)->args[0], strerror(errno));
				// exit(ERROR);
				return (-1);
			}
			if (((t_cmd *)lst_cmd->content)->status_flag & FLG_INPUT)
				flag_redir_input = 1;
			else if (((t_cmd *)lst_cmd->content)->status_flag & FLG_OUTPUT)
				flag_trunc = 1;
			else if (((t_cmd *)lst_cmd->content)->status_flag & FLG_APPEND)
				flag_append = 1;
			else
			{
				flag_redir_input = 0;
				flag_trunc = 0;
				flag_append = 0;
			}
			close(tmp_fd);
		}
		else if (!first && flag_redir_input == 1)
		{
			lst_cmd = lst_cmd->next;
			continue ;
		}
		else if (!first)
		{
			if (flag_trunc == 1)
			{
				printf("lst trunc\n");
				tmp_fd = open(((t_cmd *)lst_cmd->content)->args[0], O_WRONLY | O_TRUNC | O_CREAT, 0666);
			}
			else if (flag_append == 1)
			{
				printf("lst append\n");
				tmp_fd = open(((t_cmd *)lst_cmd->content)->args[0], O_WRONLY | O_APPEND | O_CREAT, 0666);
			}
			else if (flag_redir_input == 1)
				return (tmp_fd);
			if (tmp_fd == -1)
			{
				printf("ret\n");
				ft_dprintf(STDERR_FILENO, "minishell: %s: %s\n", ((t_cmd *)lst_cmd->content)->args[0], strerror(errno));
				// exit(ERROR);
				return (-1);
			}
			return (tmp_fd);
		}
		if (((t_cmd *)lst_cmd->content)->status_flag & FLG_OUTPUT)
		{
			flag_trunc = 1;
			flag_append = 0;
		}
		else if (((t_cmd *)lst_cmd->content)->status_flag & FLG_APPEND)
		{
			flag_trunc = 0;
			flag_append = 1;
		}
		lst_cmd = lst_cmd->next;
	}
	return (-1);
}

int  get_input_fd(t_list *lst_cmd)
{
	t_list *tmp;
	int fd;
	int tmp_fd;
	int first;

	tmp = lst_cmd;
	fd = -1;
	tmp_fd = -1;
	first = 1;
	while (tmp)
	{
		// printf("in input\n");
		if (first)
		{
			first = 0;
		}
		else if (tmp && !first)
		{
			// printf("fd opened in input : %s\n", ((t_cmd *)tmp->content)->args[0]);
			tmp_fd = open(((t_cmd *)tmp->content)->args[0], O_RDWR);
			if (tmp_fd == -1)
			{
				printf("get input fd over\n");
				if (fd == -1)
					return (-1);
				else
					return (fd);
			}
			fd = tmp_fd;
		}
		tmp = tmp->next;
	}
	return (fd);
}

int	*cmd_with_redir(void *cmd, t_list *lst_cmd)
{
	pid_t	pid;
	int		*fd = malloc(sizeof(int) * 2);
	int 	input_fd;

	if (!fd)
		return (NULL);
	input_fd = -1;
	pipe(fd);
	pid = fork();
	if (pid < 0)
			exit(1);
	else if (pid == 0)
	{
		close(fd[0]);
		input_fd = get_input_fd(lst_cmd);
		printf("input_fd : %d\n", input_fd);
		dup2(input_fd, STDIN_FILENO);
	
		fd[1] = redir_cmd(lst_cmd);
		printf("output_fd : %d\n", fd[1]);
		if (fd[1] == -1)
		{
			printf("fork fd error\n");
			// exit(0);
		}
		dup2(fd[1], STDOUT_FILENO);
	
		ft_pre_exec_cmd(cmd);
		close(fd[1]);
		exit(0);
	}
	else
	{
		close(fd[1]);
		wait(NULL);
	}
	return (fd);
}

// int	*cmd_with_redir(void *cmd, t_list *lst_cmd)
// {
// 	pid_t	pid;
// 	int		*fd = malloc(sizeof(int) * 2);

// 	if (!fd)
// 		return (NULL);
// 	pipe(fd);
// 	pid = fork();
// 	if (pid < 0)
// 			exit(1);
// 	else if (pid == 0)
// 	{
// 		fd[1] = redir_cmd(lst_cmd);
// 		if (fd[1] == -1)
// 		{
// 			printf("fork fd error\n");
// 			exit(0);
// 		}
// 		dup2(fd[1], STDOUT_FILENO);
// 		ft_pre_exec_cmd(cmd);
// 		close(fd[1]);
// 		exit(0);
// 	}
// 	else
// 	{
// 		wait(NULL);
// 	}
// 	return (fd);
// }

// int *cmd_with_redir_input(void *cmd, t_list *lst_cmd)
// {
// 	pid_t	pid;
// 	int		*fd = malloc(sizeof(int) * 2);

// 	if (!fd)
// 		return (NULL);
// 	pipe(fd);
// 	pid = fork();
// 	if (pid < 0)
// 			exit(1);
// 	else if (pid == 0)
// 	{
// 		fd[1] = redir_cmd(lst_cmd);
// 		if (fd[1] == -1)
// 		{
// 			printf("fork fd error\n");
// 			exit(0);
// 		}
// 		dup2(fd[1], STDIN_FILENO);
// 		ft_pre_exec_cmd(cmd);
// 		close(fd[1]);
// 		exit(0);
// 	}
// 	else
// 	{
// 		wait(NULL);
// 	}
// 	return (fd);
// }

void	cmd_before_get_redir(void *cmd, int *fd_from_redir)
{
	const pid_t	pid = fork();
	int		*tmp = fd_from_redir;

	if (pid < 0)
		exit(ERROR);
	else if (0 == pid)
	{
		close(fd_from_redir[1]);
		dup2(fd_from_redir[0], STDIN_FILENO);
		ft_pre_exec_cmd(cmd);
		close(fd_from_redir[0]);
		exit(0);
	}
	else
	{
		close(fd_from_redir[1]);
		wait(NULL);
	}
	free(tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////////// end direction

void	ft_exec_each_cmd(t_list *lst)
{
	t_list	*tmp;
	int		first;
	int		redir_flag;
	// int		redir_input_flag;
	int 	pipe_flag;
	int		*fd;
	
	if (!lst || !((t_cmd *)lst->content)->args)
		return ;
	tmp = lst;
	first = 1;
	redir_flag = 0;
	pipe_flag = 0;
	fd = NULL;
	while (tmp)
	{
		if ((((t_cmd *)tmp->content)->status_flag & FLG_PIPE) && first == 1)
		{
			printf("fist with pipe\n");
			fd = first_cmd_with_pipe(tmp->content);
			first = 0;
			pipe_flag = 1;
		}
		else if ((((t_cmd *)tmp->content)->status_flag & FLG_PIPE) && first == 0)
		{
			printf("interm with pipe\n");
			fd = interm_cmd_with_pipe(tmp->content, fd);
			pipe_flag = 1;
		}
		else if ((((t_cmd *)tmp->content)->status_flag & FLG_EO_CMD) && first == 0 && pipe_flag == 1)
		{
			printf("with ;\n");
			last_cmd_with_pipe(tmp->content, fd);
			pipe_flag = 0;
		}
		else if ((((t_cmd *)tmp->content)->status_flag & FLG_EOL) && pipe_flag == 1)
		{
			printf("last with pipe\n");
			last_cmd_with_pipe(tmp->content, fd);
		}
		else if (((((t_cmd *)tmp->content)->status_flag & FLG_OUTPUT) || (((t_cmd *)tmp->content)->status_flag & FLG_APPEND || (((t_cmd *)tmp->content)->status_flag & FLG_INPUT))))
		{
			printf("cmd with redir\n");
			tmp->content = get_complete_cmd(tmp->content, lst);
			fd = cmd_with_redir(tmp->content, tmp);
			redir_flag = 1;
			while (tmp && (((t_cmd *)tmp->content)->status_flag & FLG_OUTPUT || ((t_cmd *)tmp->content)->status_flag & FLG_APPEND || (((t_cmd *)tmp->content)->status_flag & FLG_INPUT)))
				tmp = tmp->next;
		}
		// else if (((t_cmd *)tmp->content)->status_flag & FLG_INPUT)
		// {
		// 	printf("cmd with redir input\n");
		// 	tmp->content = get_complete_cmd(tmp->content, lst);
		// 	fd = cmd_with_redir_input(tmp->content, tmp);
		// 	redir_input_flag = 1;
		// 	while (tmp && (((t_cmd *)tmp->content)->status_flag & FLG_INPUT))
		// 		tmp = tmp->next;
		// }
		else if (((t_cmd *)tmp->content)->args && redir_flag == 1)
		{
			printf("last with pipe before get redir\n");
			cmd_before_get_redir(tmp->content, fd);
		}
		else if (((t_cmd *)tmp->content)->args)
		{
			printf("simple\n");
			simple_cmd(tmp->content);
		}
		tmp = tmp->next;
	}
}

// void	ft_exec_each_cmd(t_list *lst)
// {
// 	t_list	*tmp;
// 	int		first;
// 	int		pipe_flag;
// 	int		*fd;
	
// 	if (!lst || !((t_cmd *)lst->content)->args)
// 		return ;
// 	tmp = lst;
// 	first = 1;
// 	pipe_flag = 0;
// 	fd = NULL;
// 	while (tmp)
// 	{
// 		if ((((t_cmd *)tmp->content)->status_flag & FLG_PIPE) && first == 1)
// 		{
// 			printf("fist with pipe\n");
// 			fd = first_cmd_with_pipe(tmp->content);
// 			first = 0;
// 			pipe_flag = 1;
// 		}
// 		else if ((((t_cmd *)tmp->content)->status_flag & FLG_PIPE) && first == 0)
// 		{
// 			printf("interm with pipe\n");
// 			fd = interm_cmd_with_pipe(tmp->content, fd);
// 			pipe_flag = 1;
// 		}
// 		else if ((((t_cmd *)tmp->content)->status_flag & FLG_EO_CMD) && first == 0 && pipe_flag == 1)
// 		{
// 			printf("with ;\n");
// 			last_cmd_with_pipe(tmp->content, fd);
// 			pipe_flag = 0;
// 		}
// 		else if ((((t_cmd *)tmp->content)->status_flag & FLG_EOL) && pipe_flag == 1)
// 		{
// 			printf("last with pipe\n");
// 			last_cmd_with_pipe(tmp->content, fd);
// 		}
// 		else if (((t_cmd *)tmp->content)->args)
// 		{
// 			printf("simple\n");
// 			simple_cmd(tmp->content);
// 		}
// 		tmp = tmp->next;
// 	}
// }

// void	ft_exec_each_cmd(void)
// {
// 	pid_t	id;
// 	id = fork();
// 	if (id < 0)
// 		exit(ERROR);
// 	if (id == 0)
// 	{
// 		multi_cmd_exec(singleton()->lst);
// 	}
// 	else
// 		wait(NULL);
// }