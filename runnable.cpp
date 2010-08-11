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

#include "runnable.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegExp>

Runnable::Runnable(const RunnableSettings &s)
	: m_stop(false), m_kill(false)
{
	/* zakážeme automatické ničení objektů Runnable, protože se o
		jejich správu staráme sami */
	setAutoDelete(false);

	// uložíme příkaz a název vstupního souboru do členské proměnné
	m_infile = s.infile;
	m_command = s.command;

	// sestavíme cestu k výstupnímu souboru...
	QFileInfo fileInfo(m_infile);
	QString outfile;

	// adresář (stejný nebo zadaný?)
	if(s.bSameDir)
		outfile = fileInfo.dir().canonicalPath() + "/";
	else
		outfile = s.outdir + "/";

	// soubor
	QString fileName = fileInfo.fileName();

	// změna přípony
	if(s.bExtension)
	{
		QRegExp regex("^(.*)\\.(\\w+)$", Qt::CaseInsensitive,
						 QRegExp::RegExp2);
		fileName.replace(regex, "\\1." + s.extension);
	}

	outfile += fileName;

	// nahradíme proměnné $FILE a $OFILE
	m_command.replace("$FILE", "\""+m_infile+"\"");
	m_command.replace("$OFILE", "\""+outfile+"\"");
}

void Runnable::run()
{
	// pokud uživatel stisknul "Zastavit" či "ZABÍT", tak ani nezačínáme
	if(m_stop)
	{
		//qDebug() << "user pressed Stop or KILL, so not even starting";
		emit destroyMe(this, false);
		return;
	}

	//qDebug() << this << "running" << m_command;

	QProcess process;

	// spustíme příkaz
	process.start(m_command, QIODevice::ReadWrite|QIODevice::Text);

	// počkáme, až se spustí
	if(process.waitForStarted(1000))
	{
		// vyšleme signál, že příkaz byl spuštěn
		emit started(m_command);
	}
	else
	{ // pokud spouštění příkazu selhalo
		// chybová hláška
		QString errorString = tr("command failed to start");

		if(process.error() != QProcess::FailedToStart)
			errorString = tr("command failed due to an unknown error");

		// vyšleme signál, že příkaz selhal
		emit failed(errorString + ": " + m_command);

		/*process.kill();
		// počkáme na zabití procesu, maximálně 1s
		process.waitForFinished(1000);*/

		emit destroyMe(this, true);

		return;
	}

	// smyčka kontrolující zda uživatel stisknul "ZABÍT"
	while(m_kill || !process.waitForFinished(200))
	{
		// pokud ne, dále čekáme na dokončení procesu
		if(!m_kill)
			continue;

		qDebug() << "killing" << m_command;

		// zabijeme proces
		process.kill();

		break;
	}

	// počkáme na zabití procesu, maximálně 1s
	process.waitForFinished(1000);

	// příkaz doběhl
	emit done(m_command, m_infile, m_kill ? 137 : process.exitCode());
	// vyžadáme zničení objektu
	emit destroyMe(this, !m_kill);
}
