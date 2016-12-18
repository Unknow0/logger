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

#include <logger.h>
#include <cfg.h>

int main()
	{
	size_t i=0;
	cfg_init(NULL);
	logger_init(NULL);

	logger_t *l=get_logger("mserver.lib");
	fatal(l, "Test: %d", i++);
	l=get_logger("mserver.lib2");
	fatal(l, "Test: %d", i++);
	l=get_logger("mserver.lib3");
	fatal(l, "Test: %d", i++);
	l=get_logger("mserver.lib3");
	fatal(l, "Test: %d", i++);

	return 0;
	}
