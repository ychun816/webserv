/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tools.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agiliber <agiliber@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 10:56:05 by agiliber          #+#    #+#             */
/*   Updated: 2024/05/31 14:33:24 by agiliber         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_putchar(char c)
{
	write(1, &c, 1);
	return (1);
}

static int	ft_size(int n)
{
	int		count;
	long	nb;

	count = 0;
	nb = n;
	if (nb < 0)
	{
		count++;
		nb *= -1;
	}
	while (nb >= 10)
	{
		nb /= 10;
		count++;
	}
	return (count + 1);
}

int	ft_putstr(char *str)
{
	int	i;

	i = 0;
	if (str == NULL)
		return (ft_putstr("(null)"), 6);
	while (str[i])
	{
		ft_putchar(str[i]);
		i++;
	}
	return (i);
}

int	ft_putnbr(int n)
{
	int		len;

	len = ft_size(n);
	if (n == -2147483648)
		return ((write(1, "-2147483648", 11)), 11);
	if (n < 0)
	{
		ft_putchar('-');
		n *= -1;
	}
	if (n >= 10)
	{
		ft_putnbr(n / 10);
		ft_putnbr(n % 10);
	}
	else
		ft_putchar(n + '0');
	return (len);
}

int	ft_putptr(void *p)
{
	unsigned long	nb;
	unsigned int	tab[256];
	int				i;
	int				len;
	char			*base;

	nb = (unsigned long)p;
	base = "0123456789abcdef";
	i = 0;
	if (nb == 0)
		return (ft_putstr("(nil)"));
	len = ft_putstr("0x");
	while (nb > 0)
	{
		tab[i] = nb % 16;
		nb /= 16;
		i++;
	}
	while (i-- > 0)
		len += ft_putchar(base[tab[i]]);
	return (len);
}