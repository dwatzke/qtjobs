/* QtJobs - a user-friendly way of multitasking
Copyright (C) 2010  David Watzke <david@watzke.cz>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <QtWidgets/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	/* this is for QSettings */
	app.setApplicationName("qtjobs");
	app.setOrganizationDomain("watzke.cz");

	/*{
	QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForCStrings(utf8);
	QTextCodec::setCodecForTr(utf8);
	}*/

	QTranslator tr;
	QString trFile = app.applicationName() + "_" + QLocale::system().name();

	tr.load(trFile);

	/* TODO: the path cannot be hardcoded, fix this */
	if(tr.isEmpty())
		tr.load(trFile, "/usr/share/qtjobs");

	app.installTranslator(&tr);

	MainWindow wnd;
	wnd.show();

	return app.exec();
}
