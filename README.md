logger
======

c logger library  
it use the libcfg to get logger configuration.

## Install
	autoreconf -is
	./configure
	make
	make install

### Logger cfg key
<table>
    <tr><td>logger.&lt;name>.format</td><td>a format string used to render record (see below)</td></tr>
    <tr><td>logger.&lt;name>.level</td><td>the level of the logger (will only log record that have a superior level) {debug=0, info, warn, error, fatal, off}</td></tr>
    <tr><td>logger.&lt;name>.str_level</td><td>array of string representation of level (used by %L)</td></tr>
    <tr><td>logger.&lt;name>.out</td><td>reconize stdout, stderr and file:&lt;path to logfile></td></tr>
</table>

you can put . in name to order logger in cfg.


### Reconized format in logger_t
these format are mostly compliant to strftime
<table>
    <tr><td>%A</td><td>Day name</td></tr>
    <tr><td>%a</td><td>Abrevied day name</td></tr>
    <tr><td>%B</td><td>Month name</td></tr>
    <tr><td>%b,%h</td><td>Abrevied month name</td></tr>
    <tr><td>%C</td><td>Century number on 2 digit</td></tr>
    <tr><td>%c</td><td>prefered datetime representation for this local</td></tr>
    <tr><td>%D</td><td>like %m/%d/%y</td></tr>
    <tr><td>%d</td><td>Day of month on 2 digit 0 padded</td></tr>
    <tr><td>%e</td><td>like %d but space padded</td></tr>
    <tr><td>%F</td><td>like %Y-%m-%d</td></tr>
    <tr><td>%H</td><td>hour ussing 24-hour clock 0 padded</td></tr>
    <tr><td>%k</td><td>like %H but space padded</td></tr>
    <tr><td>%I</td><td>hour ussing 12-hour clock 0 padded</td></tr>
    <tr><td>%l</td><td>like %I but space padded</td></tr>
    <tr><td>%j</td><td>day in the year on 3 digit 0 padded</td></tr>
    <tr><td>%M</td><td>Minute 0 padded</td></tr>
    <tr><td>%m</td><td>Month number 0 padded</td></tr>
    <tr><td>%P</td><td>pm/am</td></tr>
    <tr><td>%p</td><td>PM/AM</td></tr>
    <tr><td>%R</td><td>like %H:%M</td></tr>
    <tr><td>%r</td><td>like %I:%M:%S %p</td></tr>
    <tr><td>%S</td><td>second 0 padded</td></tr>
    <tr><td>%s</td><td>seconds since Epoch</td></tr>
    <tr><td>%T</td><td>like %H:%M:%S</td></tr>
    <tr><td>%t</td><td>tab char</td></tr>
    <tr><td>%U</td><td>week number, starting with the first Sunday</td></tr>
    <tr><td>%u</td><td>day of week 1 to 7, 1=monday</td></tr>
    <tr><td>%V</td><td>ISO 8601 week number</td></tr>
    <tr><td>%G</td><td>ISO 8601 year (four digits)</td></tr>
    <tr><td>%g</td><td>ISO 8601 year (two digits)</td></tr>
    <tr><td>%v</td><td>like %e-%b-%Y</td></tr>
    <tr><td>%W</td><td>like %U but starting with the first Monday</td></tr>
    <tr><td>%w</td><td>day of week 0 to 6, 0=sunday</td></tr>
    <tr><td>%X</td><td>prefered time representation for this local</td></tr>
    <tr><td>%x</td><td>prefered date representation for this local</td></tr>
    <tr><td>%Y</td><td>year on 4 digit</td></tr>
    <tr><td>%y</td><td>year no 2 digit</td></tr>
    <tr><td>%z</td><td>timezone offset [+-]hhmm</td></tr>
    <tr><td>%+</td><td>like %a, %d %b %Y %H:%M:%S %z</td></tr>
    <tr><td>%_</td><td>output as printf with fmt, arg passed to _l</td></tr>
    <tr><td>%L</td><td>logger level as string (from logger_t.str_level or {DBG, INF, WRN, ERR, FTL})</td></tr>
    <tr><td>%n</td><td>logger name</td></tr>
    <tr><td>%%</td><td>% char</td></tr>
</table>

