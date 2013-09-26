/*******************************************************************************
 * This file is part of Logger.
 *
 * Logger is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with Logger; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ******************************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <langinfo.h>
#include <stdarg.h>
#include <string.h>

#include <container/hashmap.h>

#include "logger.h"

#define LEAPYEAR(year) ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))


logger_t root={
	.name="root",
	.fmt="[%Y-%m-%D %H:%M:%S] %L %n: %_\n",
	.level=0,
	.str_level=NULL,
	.out=0};

static char *default_str_level[5]={"DBG", "INF", "WRN", "ERR", "FTL"};

hashmap_t *loggers;

void _fmt(int nbr, int nb, int padding)
	{
	int i;
	for(i=1; nb>1; i*=10, nb--);
	while(nbr<i)
		{
		putc(padding, root.out);
		i/=10;
		}
	while(i>0)
		{
		putc(nbr/i+'0', root.out);
		nbr%=i;
		i/=10;
		}
	}

void _log(logger_t *l, int level, char *format, ...)
	{
	int i;
	time_t ct;
	struct tm t;
	char *fmt=root.fmt;
	if(l==NULL)
		l=&root;
	if(l->level<level)
		return;
	ct=time(NULL);
	localtime_r(&ct, &t);
	for(; *fmt; ++fmt)
		{
		if(*fmt=='%')
			{
			switch(*++fmt)
				{
				case '\0':
					--fmt;
					break;
				case 'A':
					if(t.tm_wday<0 || t.tm_wday>6)
						putc('?', l->out);
					else
						fputs(nl_langinfo(DAY_1+t.tm_wday), l->out);
					continue;
				case 'a':
					if(t.tm_wday<0 || t.tm_wday>6)
						putc('?', l->out);
					else
						fputs(nl_langinfo(ABDAY_1+t.tm_wday), l->out);
					continue;
				case 'B':
					if(t.tm_mon<0 || t.tm_mon>11)
						putc('?', l->out);
					else
						fputs(nl_langinfo(MON_1+t.tm_mon), l->out);
					continue;
				case 'b':
				case 'h':
					if(t.tm_mon<0 || t.tm_mon>11)
						putc('?', l->out);
					else
						fputs(nl_langinfo(ABMON_1+t.tm_mon), l->out);
					continue;
				case 'C':
					i=t.tm_year+1900/100;
					_fmt(i, 2, '0');
					continue;
				case 'c':
					_log(l, level, "%a %b %e %H:%M:%S %Y");
					continue;
				case 'D':
					_log(l, level, "%m/%d/%y");
					continue;
				case 'd':
				case 'e':
					_fmt(t.tm_mday, 2, *fmt=='d'?'0':' ');
					continue;
				case 'F':
					_log(l, level, "%Y-%m-%d");
					continue;
				case 'H':
				case 'k':
					_fmt(t.tm_hour, 2, *fmt=='H'?'0':' ');
					continue;
				case 'I':
				case 'l':
					_fmt(t.tm_hour % 12 ? t.tm_hour % 12 : 12, 2, *fmt=='I'?'0':' ');
					continue;
				case 'j':
					_fmt(t.tm_yday+1, 3, '0');
					continue;
				case 'M':
					_fmt(t.tm_min, 2, '0');
					continue;
				case 'm':
					_fmt(t.tm_mon+1, 2, '0');
					continue;
				case 'P':
				case 'p':
					if(t.tm_hour >= 12)
						fputs(*fmt=='P'?"pm":"PM", l->out);
					else
						fputs(*fmt=='P'?"am":"AM", l->out);
					continue;
				case 'R':
					_log(l, level, "%H:%M");
					continue;
				case 'r':
					_log(l, level, "%I:%M:%S %p");
					continue;
				case 'S':
					_fmt(t.tm_sec, 2, '0');
					continue;
				case 's':
					{
					time_t mkt=mktime(&t);
					unsigned long int i;
					for(i=1; i*10<mkt; i*=10);
					while(mkt>0)
						{
						putc(mkt/i+'0', l->out);
						mkt/=10;
						i/=10;
						}
					continue;
					}
				case 'T':
					_log(l, level, "%H:%M:%S");
					continue;
				case 't':
					putc('\t', l->out);
					continue;
				case 'U':
					_fmt((t.tm_yday + 7 - t.tm_wday) / 7, 2, '0');
					continue;
				case 'u':
					_fmt(t.tm_wday == 0 ? 7 : t.tm_wday, 1, ' ');
					continue;
				case 'V':   // ISO 8601 week number
				case 'G':   // ISO 8601 year (four digits)
				case 'g': { // ISO 8601 year (two digits)
					int  year;
					int	yday;
					int	wday;
					int	w;

					year = t.tm_year + 1900;
					yday = t.tm_yday;
					wday = t.tm_wday;
					while (1)
						{
						int	len;
						int	bot;
						int	top;

						len = LEAPYEAR(year) ? 366 : 365;
						bot = ((yday + 11 - wday) % 7) - 3;
						top = bot - (len % 7);
						if (top < -3) top += 4;
						top += len;
						if (yday >= top)
							{
							++year;
							w = 1;
							break;
							}
						if (yday >= bot)
							{
							w = 1 + ((yday - bot) / 7);
							break;
							}
						--year;
						yday += LEAPYEAR(year) ? 366 : 365;
						}
					if (*fmt == 'V')
						_fmt(w, 2, '0');
					else if (*fmt == 'g')
						_fmt(year % 100, 2, '0');
					else
						_fmt(year, 4, '0');
					continue;
					}
				case 'v':
					_log(l, level, "%e-%b-%Y");
					continue;
				case 'W':
					_fmt((t.tm_yday + 7 - (t.tm_wday ? (t.tm_wday - 1) : 6)) / 7, 2, '0');
					continue;
				case 'w':
					_fmt(t.tm_wday, 1, '0');
					continue;
				case 'X':
					_log(l, level, "%H:%M:%S");
					continue;
				case 'x':
					_log(l, level, "%m/%d/%y");
					continue;
				case 'y':
					_fmt((t.tm_year+1900) % 100, 2, '0');
					continue;
				case 'Y':
					_fmt(t.tm_year+1900, 4, '0');
					continue;
				case 'z':
					{
					int diff=t.tm_gmtoff;
					if(diff<0)
						{
						diff=-diff;
						putc('-', l->out);
						}
					else 
						putc('+', l->out);
					_fmt(diff/3600, 2, '0');
					_fmt((diff%3600)/60, 2, '0');
					continue;
					}
				case '+':
					_log(l, level, "%a, %d %b %Y %H:%M:%S %z");
					continue;
				case '_':
					{
					va_list ap;
					va_start(ap, format);
					vprintf(format, ap);
					va_end(ap);
					continue;
					}
				case 'L':
					fputs(l->str_level==NULL?default_str_level[level]:l->str_level[level], l->out);
					continue;
				case 'n':
					fputs(l->name, l->out);
					continue;
				case '%':
				default:
					break;
				}
			}
		putc(*fmt, l->out);
		}
	}
size_t name_hash(void *str)
	{
	size_t h=0;
	char *c=(char *)str;
	while(*c!=0)
		h=h<2+*c++;
	return h;
	}

logger_t *get_logger(char *name)
	{
	logger_t *l;
	if(name==NULL)
		return &root;
	if(loggers==NULL)
		loggers=hashmap_create(10, &name_hash);
	l=(logger_t *)hashmap_get(loggers, name_hash(name));
	if(l!=NULL)
		return l;
	l=malloc(sizeof(logger_t));
	l->name=strdup(name);
	l->fmt=NULL; // TODO: get from cfg
	hashmap_add(loggers, l);
	}
