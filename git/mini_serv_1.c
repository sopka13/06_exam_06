/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv_1.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eyohn <sopka13@mail.ru>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/11 08:54:22 by eyohn             #+#    #+#             */
/*   Updated: 2022/02/12 14:37:04 by eyohn            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <netinet/in.h>
#include <stdio.h>

typedef struct			s_clients {
	int					fd;
	int					id;
	struct s_clients*	next;
}						t_cli;

typedef struct			s_vars {
	fd_set				all_conn;
	fd_set				rfd;
	fd_set				wfd;
	fd_set				efd;
	uint16_t			port;
	struct sockaddr_in	servaddr;
	int					sock_fd;
	// buffer
	char				buff[42 * 4096];
	// id
	int					id;
	// clients queue
	t_cli*				clients;
}						t_vars;

void	ft_error(t_vars* vars, int status){
	t_cli*	temp = vars->clients;
	while (temp){
		t_cli* tmp = temp;
		temp = temp->next;
		free(tmp);
	}
	if (status)
		write(2, "Fatal error\n", 13);
	exit(status);
}

int		max_desc(){
	return 0;
}

int		ft_start_prepare(t_vars *vars, char **argv){
	// Prepare data for server
	// return (1) - Error in system calls
	// return (0) - Fine
	FD_ZERO(&vars->all_conn);
	FD_ZERO(&vars->rfd);
	FD_ZERO(&vars->wfd);
	FD_ZERO(&vars->efd);
	vars->sock_fd = 0;
	vars->port = atoi(argv[1]);
	bzero(&vars->servaddr, sizeof(vars->servaddr));
	vars->servaddr.sin_family = AF_INET;
	vars->servaddr.sin_addr.s_addr = htonl(2130706433);
	vars->servaddr.sin_port = htons(vars->port);
	if ((vars->sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 1;
	if ((bind(vars->sock_fd, (const struct sockaddr *)&vars->servaddr, sizeof(vars->servaddr))) < 0)
		return 1;
	if (listen(vars->sock_fd, 0))
		return 1;
	FD_SET(vars->sock_fd, &vars->all_conn);

	bzero(&vars->buff, sizeof(vars->buff));
	vars->id = 0;

	vars->clients = NULL;

	return 0;
}

int		ft_send_all(int fd, t_vars *vars){
	// Send buffer all clietns and clean buffer
	t_cli *temp = vars->clients;
	
	while (temp){
		if (temp->fd != fd && FD_ISSET(temp->fd, &vars->wfd)){
			int i = send(temp->fd, vars->buff, strlen(vars->buff), 0);
			if (i < 0)
				return 1;
		}
		temp = temp->next;
	}
	bzero(&vars->buff, sizeof(vars->buff));

	return 0;
}

int		ft_add_client(t_vars *vars, int fd){
	// Add client in list
	t_cli *new = NULL;
	t_cli *temp = vars->clients;

	new = calloc(1, sizeof(t_cli));
	if (!new)
		return 1;
	else {
		if (temp == NULL){
			temp = new;
			temp->next = NULL;
		}
		while (temp->next != NULL)
			temp = temp->next;
		temp->next = new;
		new->fd = fd;
		new->id = vars->id;
		new->next = NULL;
	}
	return 0;
}

int		ft_accept(t_vars *vars){
	// This function accept connections and add in queue
	// return 0 - Success
	// return 1 - Error
	struct sockaddr_in	addr;
	socklen_t len = sizeof(addr);
	int new_id = 0;

	int fd = accept(vars->sock_fd, (struct sockaddr *) &addr, &len);
	if (fd < 0)
		return 1;
	else {
		vars->id = vars->id + 1;
		if ((new_id = ft_add_client(vars, fd)))
			return 1;
		sprintf(vars->buff, "server: client %d just arrived\n", new_id);
		ft_send_all(fd, vars);
	}
	FD_SET(fd, &vars->all_conn);
	return 0;
}

int		ft_rm_client(t_vars *vars, int fd){
	// remove client connection and return id
	t_cli *temp = vars->clients;

	while(temp){
		if (temp->next && fd == temp->next->fd){
			int ret = temp->next->id;
			t_cli *tmp = temp->next->next;
			free(temp->next);
			temp->next = tmp;
			return ret;
		}
		temp = temp->next;
	}
	return 0;
}

int		ft_get_id(t_vars *vars, int fd){
	// Search and return id for fd
	t_cli	*temp = vars->clients;

	while(temp){
		if (fd == temp->fd)
			return temp->id;
		temp = temp->next;
	}
	return 0;
}

int		ft_read(t_vars *vars, int fd){
	// Read message from fd
	// return 0 - Success
	// return 1 - Error
	int		ret = 1000;
	sprintf(vars->buff, "client %d: ", ft_get_id(vars, fd));

	while(ret == 1000){
		ret = recv(fd, vars->buff + strlen(vars->buff),
					(strlen(vars->buff) + 1000 < sizeof(vars->buff)) ? 1000 : sizeof(vars->buff) - strlen(vars->buff), 0);
		if (strlen(vars->buff) == sizeof(vars->buff))
			break;
	}
	if (ret <= 0 && strlen(vars->buff) == 0){
		bzero(&vars->buff, sizeof(vars->buff));
		sprintf(vars->buff, "server: client %d just left\n",   ft_rm_client(vars, fd));
		ft_send_all(fd, vars);
		FD_CLR(fd, &vars->all_conn);
		close(fd);
		bzero(&vars->buff, sizeof(vars->buff));
	}
	return 0;
}

int		main(int argc, char **argv){
	t_vars	vars;
	int		n;

	if (argc != 2){
		write(2, "Wrong number of arguments\n", 27);
		return 1;
	}
	if (ft_start_prepare(&vars, argv)){
		write(2, "Fatal error\n", 13);
		return 1;
	}
	while(1){
		vars.rfd = vars.wfd = vars.efd = vars.all_conn;
		if ((n = select(max_desc() + 1, &vars.rfd, &vars.wfd, &vars.efd, NULL)) < 0)
			write(2, "Select error\n", strlen("Select error\n"));
		for (int fd = 0; fd <= max_desc(); ++fd){
			if (FD_ISSET(fd, &vars.rfd)){ // need read
				if (fd == vars.sock_fd){
					if (ft_accept(&vars)){
						ft_error(&vars, 1);
					}
					break ;
				}
				ft_read(&vars, fd);
				if (strlen(vars.buff))
					ft_send_all(fd, &vars);
			}
			if (FD_ISSET(fd, &vars.efd)){ // have error
				bzero(&vars.buff, sizeof(vars.buff));
				sprintf(vars.buff, "server: client %d just left\n",   ft_rm_client(&vars, fd));
				ft_send_all(fd, &vars);
				FD_CLR(fd, &vars.all_conn);
				close(fd);
				bzero(&vars.buff, sizeof(vars.buff));
			}
		}
	}
	return (0);
}