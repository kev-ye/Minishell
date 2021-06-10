/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: besellem <besellem@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/20 22:33:29 by besellem          #+#    #+#             */
/*   Updated: 2021/06/10 11:48:24 by besellem         ###   ########.fr       */
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
	// print_prompt();
}

void	simple_cmd(void *cmd)
{
	pid_t	pid;
	int status;

	status = 1;
	if (builtin_exec(((t_cmd *)cmd)->args) == NOT_FOUND)
	{	
		pid = fork();
		if (pid < 0)
				exit(PID_FAILURE);
		else if (pid == 0)
		{
			sys_exec(cmd);
			exit(EXEC_FAILURE);
		}
		else
			wait(&status);
		if (WIFEXITED(status) != 0)
		{
			// printf("normal exit\n");
			singleton()->last_return_value = WEXITSTATUS(status);
		}
		else if (WIFSIGNALED(status) == 1)
		{
			// printf("signal exit\n");
			singleton()->last_return_value = LRV_KILL_SIG + WTERMSIG(status);
		}	
	}
}

void	ft_exec_each_cmd(t_list *lst_cmd)
{
	t_list	*tmp;
	int		cmd_line;

	tmp = lst_cmd;
	cmd_line = -1;
	if (syntax_parser(tmp))
	{
		singleton()->last_return_value = LRV_SYNTAX_ERROR;
		return ;
	}
	while (tmp)
	{
		if (((t_cmd *)tmp->content)->args
			&& ((((t_cmd *)tmp->content)->status_flag & FLG_EO_CMD)
			|| (((t_cmd *)tmp->content)->status_flag & FLG_EOL)))
		{
			printf(B_PURPLE"simple cmd"CLR_COLOR"\n");
			simple_cmd(tmp->content);
		}
		else
		{
			cmd_line = part_cmd_check(tmp);
			if (cmd_line == ONLY_PIPE)
			{
				printf(B_PURPLE"pipe cmd"CLR_COLOR"\n");
				cmd_with_pipe(tmp);
			}
			else if (cmd_line == ONLY_REDIR)
			{
				printf(B_PURPLE"redir cmd"CLR_COLOR"\n");
				cmd_with_redir(tmp->content, tmp);
			}
			else if (cmd_line == MIX)
				printf("mix\n");
		}
		while (tmp && !(((t_cmd *)tmp->content)->status_flag & FLG_EO_CMD))
			tmp = tmp->next;
		if (tmp)
			tmp = tmp->next;
	}
}