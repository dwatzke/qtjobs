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

#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	// údaje pro QSettings (konfigurák)
	app.setApplicationName("qtjobs");
	app.setOrganizationDomain("watzke.cz");

	// nastavíme kódování pro lokalizaci a řetězce C
	{
	QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForCStrings(utf8);
	QTextCodec::setCodecForTr(utf8);
	}

	// načteme lokalizaci
	QTranslator tr;
	tr.load(app.applicationName() + "_" + QLocale::system().name());
	app.installTranslator(&tr);

	// zobrazíme hlavní okno
	MainWindow wnd;
	wnd.show();

	// spustíme smyčku událostí
	return app.exec();
}
