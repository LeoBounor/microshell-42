#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void	putstr_fd(char *str, int fd)
{
	int	i;

	i = 0;
	while (str && str[i])
		write(fd, &str[i++], 1);
}

int	error_int(char *s1, char *s2)
{
	putstr_fd("error: ", 2);
	putstr_fd(s1, 2);
	putstr_fd(s2, 2);
	putstr_fd("\n", 2);
	return (1);
}

int	cd(char **argv)
{
	int	i;

	i = 0;
	while (argv[i])
		i++;
	if (i != 2)
		return (error_int("cd: bad arguments", NULL));
	if (chdir(argv[1]) == -1)
		return (error_int("cd: cannot change directory to ", argv[1]));
	return (0);
}

int	exec(char **argv, int type, int type_before, char **envp, int *pipe_save)
{
	pid_t	pid;
	int		pipex[2];
	int		child_return;

	if (type == 0 && pipe(pipex) < 0)
		return (error_int("fatal", NULL));
	pid = fork();
	if (pid < 0)
		return (error_int("fatal", NULL));
	if (pid == 0)
	{
		if (type_before == 0 && dup2(*pipe_save, 0) < 0)
			return (error_int("fatal", NULL));
		if (type == 0 && dup2(pipex[1], 1) < 0)
			return (error_int("fatal", NULL));
		if (execve(argv[0], argv, envp) < 0)
		{
			error_int("cannot execute ", argv[0]);
			exit(1);
		}
	}
	else
	{
		waitpid(pid, &child_return, 0);
		if (type_before == 0)
			close(*pipe_save);
		if (type_before == 0 && type != 0)
			close(pipex[0]);
		if (type == 0)
			close(pipex[1]);
	}
	*pipe_save = pipex[0];
	return (child_return);
}

int	main(int argc, char **argv, char **envp)
{
	int		i;
	int		type; // 0 : pipe | 1 : semicollon | 2 : end
	int		type_before;
	char	**tmp;
	int		ret;
	int		pipe_save;

	i = 1;
	int save_i = 1;
	ret = EXIT_SUCCESS;
	pipe_save = 0;
	type = 2;
	type_before = 2;
	while (i < argc)
	{
		tmp = &argv[i];
		save_i = i;
		while (i < argc && strcmp("|", argv[i]) != 0 && strcmp(";", argv[i]) != 0)
			i++;
		if (i < argc && !strcmp("|", argv[i]))
			type = 0;
		else if (i < argc && !strcmp(";", argv[i]))
			type = 1;
		else
			type = 2;
		argv[i] = 0;
		if (i != save_i && strcmp("cd", tmp[0]) == 0)
			cd(tmp);
		else if (i != save_i)
			ret = exec(tmp, type, type_before, envp, &pipe_save);
		type_before = type;
		i++;
	}
	return (ret);
}
