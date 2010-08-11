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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGui>
#include <QThreadPool>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow), m_ok(true)
{
	// načte navržené GUI
	ui->setupUi(this);

	// aktivuje první tab
	ui->tabWidget->setCurrentIndex(0);

	// nastaví počet ideální vláken do spinboxu
	ui->threadBox->setValue(QThread::idealThreadCount());

	/* za jak dlouho se vlákno zničí při nečinnosti...
		výchozích 30s je moc, protože pak se může stát, že
		program nebude respektovat nastavení počtu vláken */
	QThreadPool::globalInstance()->setExpiryTimeout(100);

	// načte profily
	loadProfiles();
}

/* nechá uživatele vybrat soubory přes souborový dialog a
	přidá je do seznamu */
void MainWindow::addFiles()
{
	QStringList files = QFileDialog::getOpenFileNames(this,
				tr("Select files to be processed"),
				QDir::homePath());
	ui->fileList->addItems(files);
}

/* odstraní vybrané soubory ze seznamu */
void MainWindow::removeSelectedFiles()
{
	QList<QListWidgetItem*> items = ui->fileList->selectedItems();

	ui->fileList->setUpdatesEnabled(false);

	while(!items.isEmpty())
		delete items.takeLast();

	ui->fileList->setUpdatesEnabled(true);
}

/* načte profily a případně vytvoří výchozí (prázdný) profil */
void MainWindow::loadProfiles(bool load_default)
{
	// ujistíme se, že comboBox je prázdný (důležité pro znovunačtení)
	while(ui->comboProfile->count())
		ui->comboProfile->removeItem(0);

	QStringList profiles = m_profiles.childGroups();

	// vytvoříme výchozí (prázdný) profil, pokud žádný neexistuje
	if(!profiles.contains("default"))
	{
		m_profiles.beginGroup("default");
		m_profiles.setValue("name", tr("Default"));
		m_profiles.setValue("command", QString());
		m_profiles.setValue("extension", QString("ext"));
		m_profiles.setValue("outdir", QDir::homePath());
		m_profiles.setValue("samedir", false);
		m_profiles.setValue("want_extension", false);
		m_profiles.setValue("autothread", true);
		m_profiles.setValue("threadcount", QThread::idealThreadCount());
		m_profiles.endGroup();

		profiles << "default";
	}

	// přidáme dostupné profily do comboBoxu
	foreach(QString profile, profiles)
	{
		m_profiles.beginGroup(profile);
		QString name = m_profiles.value("name").toString();
		m_profiles.endGroup();

		if(name.isEmpty())
			continue;

		//qDebug() << "adding profile" << profile << "as" << name;

		ui->comboProfile->addItem(name, profile);
	}

	if(load_default)
	{
		// nalistujeme v comboBoxu výchozí profil
		int index = ui->comboProfile->findData("default");
		ui->comboProfile->setCurrentIndex(index);

		// načteme jej
		loadSelectedProfile();
	}
}

/* odstraní vybraný profil */
void MainWindow::removeSelectedProfile()
{
	// zjistíme index vybraného profilu
	int index = ui->comboProfile->currentIndex();
	// a jeho název
	QString profile = ui->comboProfile->itemData(index).toString();

	if(profile.isEmpty())
		return;

	// nedovolíme smazat výchozí profil
	if(profile == "default")
	{
		QMessageBox::critical(this, tr("Error!"),
				      tr("You cannot remove the default profile!"));
		return;
	}

	// odstraníme z comboBoxu
	ui->comboProfile->removeItem(index);

	// nalistujeme výchozí profil
	index = ui->comboProfile->findData("default");
	ui->comboProfile->setCurrentIndex(index);

	// odstraníme z konfiguráku
	m_profiles.remove(profile);
}

/* načte vybraný profil */
void MainWindow::loadSelectedProfile()
{
	// zjistíme index vybraného profilu
	int index = ui->comboProfile->currentIndex();
	// a jeho název
	QString profile = ui->comboProfile->itemData(index).toString();

	// touhle dobou už bychom měli znát název profilu (pro načtení)
	Q_ASSERT(!profile.isEmpty());

	// načteme proměnné z nastavení
	m_profiles.beginGroup(profile);
	QString command = m_profiles.value("command").toString();
	QString extension = m_profiles.value("extension", "ext").toString();
	QString outdir = m_profiles.value("outdir", QDir::homePath()).toString();
	bool samedir = m_profiles.value("samedir", false).toBool();
	bool want_extension = m_profiles.value("want_extension", false).toBool();
	bool autothread = m_profiles.value("autothread", true).toBool();
	int threadcount = m_profiles.value("threadcount", QThread::idealThreadCount()).toInt();
	m_profiles.endGroup();

	// nastavíme načtené nastavení:
	// 1) příkaz
	ui->commandEdit->setText(command);

	// 2) příponu
	if(!want_extension)
		ui->extCb->setChecked(false);
	else
	{
		ui->extCb->setChecked(true);
		ui->extEdit->setText(extension);
	}

	// 3) výstupní adresář
	if(samedir)
		ui->radioSame->setChecked(true);
	else
	{
		ui->radioOther->setChecked(true);
		ui->outDirEdit->setText(outdir);
	}

	// 4) nastavení vláken
	ui->threadcountCb->setChecked(!autothread);
	ui->threadBox->setValue(threadcount);
}

/* uloží profil */
void MainWindow::saveProfile()
{
	// název profilu načteme z patřičného textového pole
	QString profile = ui->profileEdit->text();

	// pokud název nebyl zadán, přepíšeme aktuálně nalistovaný profil
	if(profile.isEmpty())
	{
		int index = ui->comboProfile->currentIndex();
		profile = ui->comboProfile->itemData(index).toString();
	}

	// touhle dobou už bychom rozhodně měli znát název profilu (pro uložení)
	Q_ASSERT(!profile.isEmpty());

	// uložíme požadované hodnoty
	m_profiles.beginGroup(profile);
	m_profiles.setValue("name", profile);
	m_profiles.setValue("command", ui->commandEdit->text());
	m_profiles.setValue("extension", ui->extEdit->text());
	m_profiles.setValue("outdir", ui->outDirEdit->text());
	m_profiles.setValue("samedir", ui->radioSame->isChecked());
	m_profiles.setValue("want_extension", ui->extCb->isChecked());
	m_profiles.setValue("autothread", !ui->threadcountCb->isChecked());
	m_profiles.setValue("threadcount", ui->threadBox->value());
	m_profiles.endGroup();

	// znovu načíst seznam profilů
	loadProfiles(false);

	// vybrat nově uložený profil
	int index = ui->comboProfile->findData(profile);
	ui->comboProfile->setCurrentIndex(index);

	// načíst nově uložený profil
	loadSelectedProfile();

	// vyprázdnit pole s názvem pro nový profil
	ui->profileEdit->setText("");
}

/* spustí příkazy */
void MainWindow::runJobs()
{
	// obnovit stav widgetů z minulého běhu
	ui->commandList->clear();
	ui->processedList->clear();
	ui->errorList->clear();
	ui->progressBar->reset();

	// vytvoříme seznam názvů souborů
	QStringList fileList;
	int count = ui->fileList->count();

	for(int i=0; i < count; ++i)
		fileList << ui->fileList->item(i)->text();

	// kontrola, zda byly zadány nějaké soubory
	if(count < 1)
	{
		QMessageBox::critical(this, tr("Error!"),
			tr("Please add some files to the file list on the \"File "
			   "list\" tab."));
		return;
	}

	// je zadána přípona?
	if(ui->extCb->isChecked() && ui->extEdit->text().isEmpty())
	{
		QMessageBox::critical(this, tr("Error!"),
			tr("Please either enter an extension or uncheck the option "
			   "for it's changing."));
		return;
	}

	// byl zadán platný výstupní adresář?
	if(ui->radioOther->isChecked())
	{
		QFileInfo dirInfo(ui->outDirEdit->text());
		if(!dirInfo.isDir() || !dirInfo.isWritable())
		{
			QMessageBox::critical(this, tr("Error!"),
				tr("Please select a valid writable output directory."));
			return;
		}
	}

	// obsahuje příkaz proměnnou $FILE?
	if(!ui->commandEdit->text().contains("$FILE"))
	{
		QMessageBox::critical(this, tr("Error!"),
			tr("The command must contain the $FILE variable (and should "
			   "also contain the optional $OFILE variable)."));
		return;
	}

	// nastavíme ukazatel průběhu
	ui->progressBar->setRange(0, count);
	ui->progressBar->setFormat("%v/%m (%p%)");
	ui->progressBar->setValue(0);

	// nastavíme počet vláken
	int threadcount = QThread::idealThreadCount();

	if(ui->threadcountCb->isChecked())
		threadcount = ui->threadBox->value();

	QThreadPool::globalInstance()->setMaxThreadCount(threadcount);

	// přepneme GUI do stavu zpracovávání
	setGuiProcessingState(true);

	// přidáme všechny příkazy do fronty...
	foreach(QString fileName, fileList)
	{
		// naše struktura s nastavením
		RunnableSettings s;

		// příkaz
		s.command = ui->commandEdit->text();
		// přípona
		s.extension = ui->extEdit->text();
		// výstupní adresář
		s.outdir = ui->outDirEdit->text();
		// vstupní soubor
		s.infile = fileName;
		// chceme příponu?
		s.bExtension = ui->extCb->isChecked();
		// chceme ukládat do adresáře, kde je uložen vstupní soubor?
		s.bSameDir = ui->radioSame->isChecked();

		Runnable *runnable = new Runnable(s);
		// propojíme signály
		connect(runnable, SIGNAL(destroyMe(Runnable*,bool)),
			this, SLOT(removeRunnable(Runnable*,bool)));
		connect(runnable, SIGNAL(started(QString)),
			this, SLOT(startedJob(QString)));
		connect(runnable, SIGNAL(done(QString,QString,int)),
			this, SLOT(doneJob(QString,QString,int)));
		connect(runnable, SIGNAL(failed(QString)),
			this, SLOT(failedJob(QString)));

		// přidáme objekt do seznamu
		m_runnables << runnable;

		// a přidáme job do threadpoolu
		QThreadPool::globalInstance()->start(runnable);
	}

	// přepnout na tab s průběhem
	ui->tabWidget->setCurrentIndex(2);
}

/* zastaví příkazy */
void MainWindow::stopJobs()
{
	foreach(Runnable *runnable, m_runnables)
		runnable->stop();
}

/* zabije příkazy */
void MainWindow::killJobs()
{
	foreach(Runnable *runnable, m_runnables)
		runnable->kill();
}

/* odstraní dokončený příkaz (instanci Runnable) */
void MainWindow::removeRunnable(Runnable *runnable, bool ok)
{
	m_runnables.removeAll(runnable);
	delete runnable;

	if(!ok)
		m_ok = false;

	checkProcessing();
}

/* slot pro "příkaz byl spuštěn" */
void MainWindow::startedJob(QString cmd)
{
	// přidá příkaz do seznamu příkazů
	new QListWidgetItem(cmd, ui->commandList);
}

/* slot pro "příkaz byl dokončen" (s názvem souboru a návratovou hodnotou) */
void MainWindow::doneJob(QString cmd, QString file, int retcode)
{
	// aktualizujeme hodnotu v ukazateli průběhu
	int done = ui->progressBar->value()+1;
	ui->progressBar->setValue(done);

	/* podle návratové hodnoty přidáme soubor do seznamu úspěšně zpracovaných
	   nebo přidáme příkaz do seznamu chyb */
	if(!retcode)
		ui->processedList->addItem(file);
	else
	{
		QString error = QString(tr("command returned %1: %2")).arg(retcode).arg(cmd);
		ui->errorList->addItem(error);
	}

	// najdeme v seznamu běžících příkazů ten, který právě skončil
	QList<QListWidgetItem*> item = ui->commandList->findItems(cmd,
						Qt::MatchExactly|Qt::MatchCaseSensitive);

	//Q_ASSERT(item.count() == 1);

	// ověříme, zda jsme nalezli jen 1
	if(item.count() != 1)
	{
		qDebug() << "WARNING: removeCommandFromList() found >1 item."
			 << "This shouldn't happen. Items:";
		foreach(QListWidgetItem* it, item)
			qDebug() << "> " << it->text();
	}

	// a smažeme jej
	if(item.count() > 0)
		delete item.at(0);
	else
		qDebug() << "BUG: removeCommandFromList() found no items."
			 << "This should never happen.";
}

/* slot pro "příkaz selhal" */
void MainWindow::failedJob(QString error)
{
	// aktualizujeme hodnotu v ukazateli průběhu
	int done = ui->progressBar->value()+1;
	ui->progressBar->setValue(done);

	// přidáme do seznamu chyb
	ui->errorList->addItem(error);
}

/* přepne GUI do stavu zpracovávání (nebo naopak) */
void MainWindow::setGuiProcessingState(bool processing)
{
	ui->runBtn->setDisabled(processing);
	ui->stopBtn->setEnabled(processing);
	ui->killBtn->setEnabled(processing);

	if(processing)
		m_ok = true;
}

/* zkontroluje, zda už je vše hotovo a ukáže patřičnou zprávu */
void MainWindow::checkProcessing()
{
	// pokud v seznamu jsou ještě úlohy, tak hned vrátíme
	if(m_runnables.count())
		return;

	// zdá se, že máme hotovo

	if(QThreadPool::globalInstance()->activeThreadCount() != 0)
		qDebug() << "looks like processing is done but"
			 << "QThreadPool::activeThreadCount() != 0";

	// vrátíme GUI do klidového stavu
	setGuiProcessingState(false);

	// vyprázdníme seznam příkazů (pro jistotu)
	ui->commandList->clear();
	//ui->processedList->clear();

	// hotovo bez chyb
	if(m_ok && !ui->errorList->count())
		QMessageBox::information(this, tr("Done!"),
			tr("Done! All the files were successfully processed!"));
	else if(m_ok)
	{ // hotovo s chybami
		QMessageBox::StandardButton btn;
		btn = QMessageBox::warning(this, tr("Done with errors!"),
				tr("Done with errors! See the list of the failed "
				   "commands on the \"Progress\" tab."));
		if(btn == QMessageBox::Ok)
			ui->tabWidget->setCurrentIndex(2);
	} // přerušeno uživatelem
	else
		QMessageBox::critical(this, tr("Interrupted!"),
			tr("Processing was interrupted by pressing the Stop or "
			   "the KILL button."));

	// resetujeme ukazatel průběhu
	ui->progressBar->reset();
}

/* vynutí změnu přípony
	-> hodí se (nejen) když výstupní soubory ukládáte do stejného adresáře */
void MainWindow::forceExtensionChange(bool force)
{
	// pokud změnu přípony chceme vynutit, tak zaškrtneme příslušné pole
	if(force)
		ui->extCb->setChecked(true);

	// povolí nebo zakáže změnu zaškrtávacího pole
	ui->extCb->setDisabled(force);
}

/* výchozí destruktor */
MainWindow::~MainWindow()
{
	delete ui;
}

/* tuhle funkci vygeneroval Qt Creator sám;
 umožňuje měnit lokalizaci za běhu... */
void MainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);

	switch(e->type())
	{
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
