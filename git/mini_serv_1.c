/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv_1.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eyohn <sopka13@mail.ru>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/11 08:54:22 by eyohn             #+#    #+#             */
/*   Updated: 2022/02/11 11:45:32 by eyohn            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/select.h>

int		max_desc(){
	
}

int		ft_prepare(){
	// Prepare data for server
	// return (1) - Error in system calls
	// return (0) - Fine
}

int		main(int argc, char* argv){
	int n;

	if (argc != 2){
		write(2, "Wrong number of arguments\n", 27);
		return 1;
	}
	if (ft_prepare()){
		write(2, "Fatal error\n", 13);
		return 1;
	}
	while(1){
		switch (n = select(1 + max(console, serial), &rfd, &wfd, NULL, NULL)){
			case -1:
				write(2, "Select error\n", strlen("Select error\n"));
				continue ;
			case 0:
				continue ;
			default:
				if (FD_ISSET())
		}
	}
}