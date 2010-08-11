QtJobs - a user-friendly way of multitasking
(c) 2010  David Watzke <david@watzke.cz>

This project is released under the GPLv3+ license. You can read the license
text in the file called LICENSE that's located next to this README.

---
To build the project, run:
	lrelease qtjobs.pro
	qmake # or qmake-qt4
	make

This creates a native binary called qtjobs that you can run like so:
	./qtjobs

If you want to install the program system-wide, so far it's enough to just
copy this binary to /usr/bin/ or somewhere else in the PATH, but this can
change, so watch out for it.

---
If you want to help by contributing, you can create a new translation.
To do that, copy the Czech one and re-translate it. For example a German
translator would run this:
	cp qtjobs_cs_CZ.ts qtjobs_de_DE.ts
	linguist qtjobs_de_DE.ts
and start translating.

The second command runs the Qt Linguist program that allows you to translate
the software easily and checks the translation for errors that could cause
problems, so please send only translations without warnings from Linguist.

---
Another way of helping the project is packaging it for various systems, such
as GNU/Linux distributions and possibly even other OSes.

---
If you found a bug, please make sure that you're using the latest version
from the Git repository git://repo.or.cz/qtjobs.git and if so, then please
don't hesitate to report the bug to me with full description and the OS info.

Also, if you'd like to request a new feature, please do so.

My e-mail address is stated at the beginning of this file.
