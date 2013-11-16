logger
======

c logger library
it use the libcfg to get logger configuration.

### Logger cfg key
logger.<name>.format	a format string used to render record (see below)
logger.<name>.level		the level of the logger (will only log record that have a superior level) {debug=0, info, warn, error, fatal, off}
logger.<name>.str_level	array of string representation of level (used by %L)
logger.<name>.out		reconize stdout, stderr and file:<path to logfile>

you can put . in name to order logger in cfg.


### Reconized format in logger_t
these format are mostly compliant to strftime
	%A	Day name
	%a	Abrevied day name
	%B	Month name
	%b,%h	Abrevied month name
	%C	Century number on 2 digit
	%c	prefered datetime representation for this local
	%D	like %m/%d/%y
	%d	Day of month on 2 digit 0 padded
	%e	like %d but space padded
	%F	like %Y-%m-%d
	%H	hour ussing 24-hour clock 0 padded
	%k	like %H but space padded
	%I	hour ussing 12-hour clock 0 padded
	%l	like %I but space padded
	%j	day in the year on 3 digit 0 padded
	%M	Minute 0 padded
	%m	Month number 0 padded
	%P	pm/am
	%p	PM/AM
	%R	like %H:%M
	%r	like %I:%M:%S %p
	%S	second 0 padded
	%s	seconds since Epoch
	%T	like %H:%M:%S
	%t	tab char
	%U	week number	starting with the first Sunday
	%u	day of week 1 to 7, 1=monday
	%V	ISO 8601 week number
	%G	ISO 8601 year (four digits)
	%g	ISO 8601 year (two digits)
	%v	like %e-%b-%Y
	%W	like %U but starting with the first Monday
	%w	day of week 0 to 6, 0=sunday
	%X	prefered time representation for this local
	%x	prefered date representation for this local
	%Y	year on 4 digit
	%y	year no 2 digit
	%z	timezone offset [+-]hhmm
	%+	like %a, %d %b %Y %H:%M:%S %z
	%_	output as printf with fmt, arg passed to _l
	%L	logger level as string (from logger_t.str_level or {DBG, INF, WRN, ERR, FTL})
	%n	logger name
	%%	% char


### TODO
hierachical logger
