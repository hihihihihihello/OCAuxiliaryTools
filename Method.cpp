#include "Method.h"

#include "filesystemwatcher.h"
#include "mainwindow.h"
#include "plistparser.h"
#include "plistserializer.h"
#include "ui_mainwindow.h"

extern MainWindow* mw_one;
Method* mymethod;

QString strACPI;
QString strKexts;
QString strDrivers;
QString strTools;

Method::Method(QWidget* parent) : QMainWindow(parent) { mymethod = new Method; }

QObjectList Method::getAllToolButton(QObjectList lstUIControls) {
  QObjectList lstOfToolButton;
  foreach (QObject* obj, lstUIControls) {
    if (obj->metaObject()->className() == QStringLiteral("QToolButton")) {
      lstOfToolButton.append(obj);
    }
  }
  return lstOfToolButton;
}

QString Method::loadText(QString textFile) {
  QFileInfo fi(textFile);
  if (fi.exists()) {
    QFile file(textFile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(
          this, tr("Application"),
          tr("Cannot read file %1:\n%2.")
              .arg(QDir::toNativeSeparators(textFile), file.errorString()));

    } else {
      QTextStream in(&file);
      in.setCodec("UTF-8");
      QString text = in.readAll();
      return text;
    }
  }

  return "";
}

QString Method::getKextVersion(QString kextFile) {
  QString strInfo = kextFile + "/Contents/Info.plist";
  QTextEdit* txtEdit = new QTextEdit;
  txtEdit->setPlainText(loadText(strInfo));
  for (int i = 0; i < txtEdit->document()->lineCount(); i++) {
    QString str0 = getTextEditLineText(txtEdit, i).trimmed();
    str0.replace("</key>", "");
    str0.replace("<key>", "");
    if (str0 == "CFBundleVersion") {
      QString str1 = getTextEditLineText(txtEdit, i + 1).trimmed();
      str1.replace("<string>", "");
      str1.replace("</string>", "");
      return str1;
    }
  }

  return "";
}

QString Method::getTextEditLineText(QTextEdit* txtEdit, int i) {
  QTextBlock block = txtEdit->document()->findBlockByNumber(i);
  txtEdit->setTextCursor(QTextCursor(block));
  QString lineText = txtEdit->document()->findBlockByNumber(i).text().trimmed();
  return lineText;
}

bool Method::isKext(QString kextName) {
  QString str = kextName.mid(kextName.length() - 4, 4);
  // qDebug() << str;
  if (str == "kext")
    return true;
  else
    return false;
}

QString Method::getKextBin(QString kextName) {
  QString str0 = kextName.mid(0, kextName.length() - 5);
  QStringList tempList = str0.split("/");
  QString str1 = tempList.at(tempList.count() - 1);
  QString str2 = kextName + "/Contents/MacOS/" + str1;
  // qDebug() << str0 << str1 << str2;
  return str2;
}

QString Method::getMD5(QString targetFile) {
  QCryptographicHash hashTest(QCryptographicHash::Md5);
  QFile f2(targetFile);
  f2.open(QFile::ReadOnly);
  hashTest.reset();  // 重置（很重要）
  hashTest.addData(&f2);
  QString targetHash = hashTest.result().toHex();
  f2.close();
  return targetHash;
}

void Method::setStatusBarTip(QWidget* w) {
  QString strStatus0 = w->toolTip();
  QString strStatus1;
  QStringList strList = strStatus0.split("----");
  if (strList.count() == 2) {
    QTextEdit* tempEdit = new QTextEdit;
    QLocale locale;
    if (locale.language() == QLocale::Chinese) {
      tempEdit->setText(strList.at(1));
      for (int m = 0; m < tempEdit->document()->lineCount(); m++) {
        QTextBlock block = tempEdit->document()->findBlockByNumber(m);
        tempEdit->setTextCursor(QTextCursor(block));
        QString lineText =
            tempEdit->document()->findBlockByNumber(m).text().trimmed();
        if (lineText.mid(0, 2) == "描述" || lineText.mid(0, 2) == "说明") {
          strStatus1 = lineText;
          break;
        }
      }

    } else {
      tempEdit->setText(strList.at(0));
      for (int m = 0; m < tempEdit->document()->lineCount(); m++) {
        QTextBlock block = tempEdit->document()->findBlockByNumber(m);
        tempEdit->setTextCursor(QTextCursor(block));
        QString lineText =
            tempEdit->document()->findBlockByNumber(m).text().trimmed();
        if (lineText.mid(0, 11) == "Description") {
          strStatus1 = lineText;
          break;
        }
      }
    }
  } else
    strStatus1 = strStatus0;
  w->setStatusTip(strStatus1);
}

void Method::set_nv_key(QString key, QString dataType) {
  bool re = false;

  for (int i = 0; i < mw_one->ui->table_nv_add->rowCount(); i++) {
    QString str;
    str = mw_one->ui->table_nv_add->item(i, 0)->text();
    if (str == key) {
      mw_one->ui->table_nv_add->setCurrentCell(i, 0);
      re = true;
    }
  }

  if (!re) {
    mw_one->on_btnNVRAMAdd_Add_clicked();

    mw_one->ui->table_nv_add->setItem(mw_one->ui->table_nv_add->rowCount() - 1,
                                      0, new QTableWidgetItem(key));

    QTableWidgetItem* newItem1 = new QTableWidgetItem(dataType);
    newItem1->setTextAlignment(Qt::AlignCenter);
    mw_one->ui->table_nv_add->setItem(mw_one->ui->table_nv_add->rowCount() - 1,
                                      1, newItem1);

    //保存数据
    mw_one->write_ini(mw_one->ui->table_nv_add0, mw_one->ui->table_nv_add,
                      mw_one->ui->table_nv_add0->currentRow());
  }
}

QWidget* Method::getSubTabWidget(int m, int s) {
  for (int j = 0; j < mw_one->mainTabList.count(); j++) {
    if (j == m) {
      for (int i = 0; i < mw_one->mainTabList.at(j)->tabBar()->count(); i++) {
        if (i == s) return mw_one->mainTabList.at(j)->widget(i);
      }
    }
  }

  return NULL;
}

void Method::goACPITable(QTableWidget* table) {
  // ACPI
  if (table == mw_one->ui->table_acpi_add) {
    mw_one->ui->listMain->setCurrentRow(0);
    mw_one->ui->listSub->setCurrentRow(0);
  }

  if (table == mw_one->ui->table_acpi_del) {
    mw_one->ui->listMain->setCurrentRow(0);
    mw_one->ui->listSub->setCurrentRow(1);
  }

  if (table == mw_one->ui->table_acpi_patch) {
    mw_one->ui->listMain->setCurrentRow(0);
    mw_one->ui->listSub->setCurrentRow(2);
  }
}

void Method::goBooterTable(QTableWidget* table) {
  // Booter
  if (table == mw_one->ui->table_booter) {
    mw_one->ui->listMain->setCurrentRow(1);
    mw_one->ui->listSub->setCurrentRow(0);
  }
  if (table == mw_one->ui->table_Booter_patch) {
    mw_one->ui->listMain->setCurrentRow(1);
    mw_one->ui->listSub->setCurrentRow(1);
  }
}

void Method::goDPTable(QTableWidget* table) {
  // DP
  if (table == mw_one->ui->table_dp_add0) {
    mw_one->ui->listMain->setCurrentRow(2);
    mw_one->ui->listSub->setCurrentRow(0);
  }
  if (table == mw_one->ui->table_dp_add) {
    mw_one->ui->listMain->setCurrentRow(2);
    mw_one->ui->listSub->setCurrentRow(0);
  }
  if (table == mw_one->ui->table_dp_del0) {
    mw_one->ui->listMain->setCurrentRow(2);
    mw_one->ui->listSub->setCurrentRow(1);
  }
  if (table == mw_one->ui->table_dp_del) {
    mw_one->ui->listMain->setCurrentRow(2);
    mw_one->ui->listSub->setCurrentRow(1);
  }
}

void Method::goKernelTable(QTableWidget* table) {
  // Kernel
  if (table == mw_one->ui->table_kernel_Force) {
    mw_one->ui->listMain->setCurrentRow(3);
    mw_one->ui->listSub->setCurrentRow(2);
  }

  if (table == mw_one->ui->table_kernel_add) {
    mw_one->ui->listMain->setCurrentRow(3);
    mw_one->ui->listSub->setCurrentRow(0);
  }

  if (table == mw_one->ui->table_kernel_block) {
    mw_one->ui->listMain->setCurrentRow(3);
    mw_one->ui->listSub->setCurrentRow(1);
  }

  if (table == mw_one->ui->table_kernel_patch) {
    mw_one->ui->listMain->setCurrentRow(3);
    mw_one->ui->listSub->setCurrentRow(3);
  }
}

void Method::goMiscTable(QTableWidget* table) {
  // Misc
  if (table == mw_one->ui->tableBlessOverride) {
    mw_one->ui->listMain->setCurrentRow(4);
    mw_one->ui->listSub->setCurrentRow(3);
  }

  if (table == mw_one->ui->tableEntries) {
    mw_one->ui->listMain->setCurrentRow(4);
    mw_one->ui->listSub->setCurrentRow(4);
  }

  if (table == mw_one->ui->tableTools) {
    mw_one->ui->listMain->setCurrentRow(4);
    mw_one->ui->listSub->setCurrentRow(5);
  }
}

void Method::goNVRAMTable(QTableWidget* table) {
  // NVRAM
  if (table == mw_one->ui->table_nv_add0) {
    mw_one->ui->listMain->setCurrentRow(5);
    mw_one->ui->listSub->setCurrentRow(0);
  }

  if (table == mw_one->ui->table_nv_add) {
    mw_one->ui->listMain->setCurrentRow(5);
    mw_one->ui->listSub->setCurrentRow(0);
  }

  if (table == mw_one->ui->table_nv_del0) {
    mw_one->ui->listMain->setCurrentRow(5);
    mw_one->ui->listSub->setCurrentRow(1);
  }

  if (table == mw_one->ui->table_nv_del) {
    mw_one->ui->listMain->setCurrentRow(5);
    mw_one->ui->listSub->setCurrentRow(1);
  }

  if (table == mw_one->ui->table_nv_ls0) {
    mw_one->ui->listMain->setCurrentRow(5);
    mw_one->ui->listSub->setCurrentRow(2);
  }

  if (table == mw_one->ui->table_nv_ls) {
    mw_one->ui->listMain->setCurrentRow(5);
    mw_one->ui->listSub->setCurrentRow(2);
  }
}

void Method::goTable(QTableWidget* table) {
  goACPITable(table);

  goBooterTable(table);

  goDPTable(table);

  goKernelTable(table);

  goMiscTable(table);

  goNVRAMTable(table);

  // PI
  if (table == mw_one->ui->tableDevices) {
    mw_one->ui->listMain->setCurrentRow(6);
    mw_one->ui->listSub->setCurrentRow(2);
  }

  // UEFI
  if (table == mw_one->ui->table_uefi_drivers) {
    mw_one->ui->listMain->setCurrentRow(7);
    mw_one->ui->listSub->setCurrentRow(3);
  }

  if (table == mw_one->ui->table_uefi_ReservedMemory) {
    mw_one->ui->listMain->setCurrentRow(7);
    mw_one->ui->listSub->setCurrentRow(8);
  }
}

QString Method::copyDrivers(QString pathSource, QString pathTarget) {
  // OC/Drivers

  QDir dir;
  QString strDatabase;
  QString pathOCDrivers = pathTarget + "OC/Drivers/";
  if (dir.mkpath(pathOCDrivers)) {
  }
  for (int i = 0; i < mw_one->ui->table_uefi_drivers->rowCount(); i++) {
    QString file = mw_one->ui->table_uefi_drivers->item(i, 0)->text();
    QString str0 = pathSource + "EFI/OC/Drivers/" + file;
    if (!str0.contains("#")) {
      QFileInfo fi(str0);
      if (fi.exists())
        QFile::copy(str0, pathOCDrivers + file);
      else
        strDatabase = strDatabase + "EFI/OC/Drivers/" + file + "\n";
    }
  }

  return strDatabase;
}

QString Method::copyKexts(QString pathSource, QString pathTarget) {
  // OC/Kexts

  QDir dir;
  QString strDatabase;
  QString pathOCKexts = pathTarget + "OC/Kexts/";
  if (dir.mkpath(pathOCKexts)) {
  }
  for (int i = 0; i < mw_one->ui->table_kernel_add->rowCount(); i++) {
    QString file = mw_one->ui->table_kernel_add->item(i, 0)->text();
    QString str0 = pathSource + "EFI/OC/Kexts/" + file;
    QDir kextDir(str0);

    if (!str0.contains("#")) {
      if (kextDir.exists())
        mw_one->copyDirectoryFiles(str0, pathOCKexts + file, true);
      else
        strDatabase = strDatabase + "EFI/OC/Kexts/" + file + "\n";
    }
  }

  return strDatabase;
}

QString Method::copyACPI(QString pathSource, QString pathTarget) {
  // OC/ACPI

  QDir dir;
  QString strDatabase;
  QString pathOCACPI = pathTarget + "OC/ACPI/";
  if (dir.mkpath(pathOCACPI)) {
  }

  for (int i = 0; i < mw_one->ui->table_acpi_add->rowCount(); i++) {
    QString file = mw_one->ui->table_acpi_add->item(i, 0)->text();
    QFileInfo fi(pathSource + "EFI/OC/ACPI/" + file);
    if (fi.exists())
      QFile::copy(pathSource + "EFI/OC/ACPI/" + file, pathOCACPI + file);
    else
      strDatabase = strDatabase + "EFI/OC/ACPI/" + file + "\n";
  }

  return strDatabase;
}

QString Method::copyTools(QString pathSource, QString pathTarget) {
  // OC/Tools

  QDir dir;
  QString strDatabase;
  QString pathOCTools = pathTarget + "OC/Tools/";
  if (dir.mkpath(pathOCTools)) {
  }
  for (int i = 0; i < mw_one->ui->tableTools->rowCount(); i++) {
    QString file = mw_one->ui->tableTools->item(i, 0)->text();
    QString str0 = pathSource + "EFI/OC/Tools/" + file;
    if (!str0.contains("#")) {
      QFileInfo fi(str0);
      if (fi.exists())
        QFile::copy(str0, pathOCTools + file);
      else
        strDatabase = strDatabase + "EFI/OC/Tools/" + file + "\n";
    }
  }

  return strDatabase;
}

void Method::on_GenerateEFI() {
  QDir dir;
  QString strDatabase;

  QFileInfo appInfo(qApp->applicationDirPath());
  QString pathSource = appInfo.filePath() + "/Database/";

  QString pathTarget = QDir::homePath() + "/Desktop/EFI/";

  mw_one->deleteDirfile(pathTarget);

  if (dir.mkpath(pathTarget)) {
  }

  // BOOT
  QString pathBoot = pathTarget + "BOOT/";
  if (dir.mkpath(pathBoot)) {
  }
  QFile::copy(pathSource + "EFI/BOOT/BOOTx64.efi", pathBoot + "BOOTx64.efi");

  // ACPI
  strDatabase = copyACPI(pathSource, pathTarget) + strDatabase;

  // Drivers
  strDatabase = copyDrivers(pathSource, pathTarget) + strDatabase;

  // Kexts
  strDatabase = copyKexts(pathSource, pathTarget) + strDatabase;

  // OC/Resources
  QString pathOCResources = pathTarget + "OC/Resources/";
  mw_one->copyDirectoryFiles(pathSource + "EFI/OC/Resources/", pathOCResources,
                             true);

  // Tools
  strDatabase = copyTools(pathSource, pathTarget) + strDatabase;

  // OC/OpenCore.efi
  QFile::copy(pathSource + "EFI/OC/OpenCore.efi",
              pathTarget + "OC/OpenCore.efi");

  // OC/Config.plist
  mw_one->SavePlist(pathTarget + "OC/Config.plist");

  QMessageBox box;
  if (strDatabase != "")
    box.setText(tr("Finished generating the EFI folder on the desktop.") +
                "\n" +
                tr("The following files do not exist in the database at the "
                   "moment, please add them yourself:") +
                "\n" + strDatabase);
  else
    box.setText(tr("Finished generating the EFI folder on the desktop."));

  mw_one->setFocus();
  box.exec();
  mw_one->ui->cboxFind->setFocus();
}

void Method::on_btnExportMaster() {
  QFileDialog fd;
  QString defname;
  int index = mw_one->ui->tabTotal->currentIndex();

  defname = getTabTextName(index);

  QString FileName =
      fd.getSaveFileName(this, tr("Save File"), defname,
                         tr("Config file(*.plist);;All files(*.*)"));
  if (FileName.isEmpty()) return;

  QVariantMap OpenCore;

  switch (index) {
    case 0:
      OpenCore["ACPI"] = mw_one->SaveACPI();

      break;

    case 1:
      OpenCore["Booter"] = mw_one->SaveBooter();
      break;

    case 2:
      OpenCore["DeviceProperties"] = mw_one->SaveDeviceProperties();
      break;

    case 3:
      OpenCore["Kernel"] = mw_one->SaveKernel();
      break;

    case 4:
      OpenCore["Misc"] = mw_one->SaveMisc();
      break;

    case 5:
      OpenCore["NVRAM"] = mw_one->SaveNVRAM();
      break;

    case 6:
      OpenCore["PlatformInfo"] = mw_one->SavePlatformInfo();
      break;

    case 7:
      OpenCore["UEFI"] = mw_one->SaveUEFI();
      break;
  }

  PListSerializer::toPList(OpenCore, FileName);
}

QString Method::getTabTextName(int index) {
  for (int i = 0; i < mw_one->ui->tabTotal->tabBar()->count(); i++) {
    if (i == index) {
      return mw_one->ui->tabTotal->tabText(index) + ".plist";
      break;
    }
  }

  return "";
}

void Method::on_btnImportMaster() {
  QFileDialog fd;
  QString defname;
  int index = mw_one->ui->tabTotal->currentIndex();
  defname = getTabTextName(index);

  QString FileName =
      fd.getOpenFileName(this, tr("Open File"), defname,
                         tr("Config file(*.plist);;All files(*.*)"));
  if (FileName.isEmpty()) return;

  mw_one->loading = true;

  QFile file(FileName);
  QVariantMap map = PListParser::parsePList(&file).toMap();

  switch (index) {
    case 0:
      // ACPI
      init_Table(0);

      mw_one->ParserACPI(map);

      break;

    case 1:
      // Booter
      init_Table(1);

      mw_one->ParserBooter(map);
      break;

    case 2:
      // DP
      init_Table(2);

      mw_one->ParserDP(map);
      break;

    case 3:
      // Kernel
      init_Table(3);

      mw_one->ParserKernel(map);
      break;

    case 4:
      // Misc
      init_Table(4);

      mw_one->ParserMisc(map);
      break;

    case 5:
      // NVRAM
      init_Table(5);

      mw_one->ParserNvram(map);
      break;

    case 6:
      // PI
      init_Table(6);

      mw_one->ParserPlatformInfo(map);
      break;

    case 7:
      // UEFI
      init_Table(7);

      mw_one->ParserUEFI(map);
      break;
  }

  mw_one->loading = false;
}

void Method::init_Table(int index) {
  if (index == -1) {
    mw_one->listOfTableWidget.clear();
    mw_one->listOfTableWidget = mw_one->getAllTableWidget(
        mw_one->getAllUIControls(mw_one->ui->tabTotal));
    for (int i = 0; i < mw_one->listOfTableWidget.count(); i++) {
      QTableWidget* w = (QTableWidget*)mw_one->listOfTableWidget.at(i);

      w->setRowCount(0);
    }
  } else {
    for (int i = 0; i < mw_one->ui->tabTotal->tabBar()->count(); i++) {
      if (index == i) {
        mw_one->listOfTableWidget.clear();
        mw_one->listOfTableWidget = mw_one->getAllTableWidget(
            mw_one->getAllUIControls(mw_one->ui->tabTotal->widget(i)));
        for (int j = 0; j < mw_one->listOfTableWidget.count(); j++) {
          QTableWidget* w = (QTableWidget*)mw_one->listOfTableWidget.at(j);

          w->setRowCount(0);
        }
      }
    }
  }
}

void Method::findDP(QTableWidget* t, QString findText) {
  // DP
  if (t == mw_one->ui->table_dp_add0) {
    if (t->rowCount() > 0) {
      for (int j = 0; j < t->rowCount(); j++) {
        t->setCurrentCell(j, 0);
        mw_one->findTable(mw_one->ui->table_dp_add, findText);
      }
    }
  }

  if (t == mw_one->ui->table_dp_del0) {
    if (t->rowCount() > 0) {
      for (int j = 0; j < t->rowCount(); j++) {
        t->setCurrentCell(j, 0);
        mw_one->findTable(mw_one->ui->table_dp_del, findText);
      }
    }
  }
}

void Method::findNVRAM(QTableWidget* t, QString findText) {
  // NVRAM
  if (t == mw_one->ui->table_nv_add0) {
    if (t->rowCount() > 0) {
      for (int j = 0; j < t->rowCount(); j++) {
        t->setCurrentCell(j, 0);
        mw_one->findTable(mw_one->ui->table_nv_add, findText);
      }
    }
  }

  if (t == mw_one->ui->table_nv_del0) {
    if (t->rowCount() > 0) {
      for (int j = 0; j < t->rowCount(); j++) {
        t->setCurrentCell(j, 0);
        mw_one->findTable(mw_one->ui->table_nv_del, findText);
      }
    }
  }

  if (t == mw_one->ui->table_nv_ls0) {
    if (t->rowCount() > 0) {
      for (int j = 0; j < t->rowCount(); j++) {
        t->setCurrentCell(j, 0);
        mw_one->findTable(mw_one->ui->table_nv_ls, findText);
      }
    }
  }
}

void Method::findTable(QString findText) {
  // Table  2
  mw_one->listOfTableWidget.clear();
  mw_one->listOfTableWidget =
      mw_one->getAllTableWidget(mw_one->getAllUIControls(mw_one->ui->tabTotal));
  mw_one->listOfTableWidgetResults.clear();
  for (int i = 0; i < mw_one->listOfTableWidget.count(); i++) {
    QTableWidget* t;
    t = (QTableWidget*)mw_one->listOfTableWidget.at(i);

    // DP
    findDP(t, findText);

    // NVRAM
    findNVRAM(t, findText);

    if (t != mw_one->ui->table_dp_add && t != mw_one->ui->table_dp_del &&
        t != mw_one->ui->table_nv_add && t != mw_one->ui->table_nv_del &&
        t != mw_one->ui->table_nv_ls)
      mw_one->findTable(t, findText);
  }
}

void Method::UpdateStatusBarInfo() {
  QObjectList listTable;
  QTableWidget* t;

  QWidget* w = getSubTabWidget(mw_one->ui->listMain->currentRow(),
                               mw_one->ui->listSub->currentRow());
  listTable = mw_one->getAllTableWidget(mw_one->getAllUIControls(w));

  if (listTable.count() > 1) {
    for (int i = 0; i < listTable.count(); i++) {
      t = (QTableWidget*)listTable.at(i);
      if (!t->currentIndex().isValid()) return;
      if (t->hasFocus()) t->cellClicked(t->currentRow(), t->currentColumn());
    }
  } else if (listTable.count() == 1) {
    t = (QTableWidget*)listTable.at(0);
    if (!t->currentIndex().isValid()) return;
    t->cellClicked(t->currentRow(), t->currentColumn());
  }
}

void Method::addFileSystemWatch(QString strOpenFile) {
  QString strPath;

  QFileInfo fi(strOpenFile);

  strPath = fi.path();
  strACPI = strPath + "/ACPI/";
  strKexts = strPath + "/Kexts/";
  strDrivers = strPath + "/Drivers/";
  strTools = strPath + "/Tools/";

  FileSystemWatcher::addWatchPath(strACPI);
  FileSystemWatcher::addWatchPath(strKexts);
  FileSystemWatcher::addWatchPath(strDrivers);
  FileSystemWatcher::addWatchPath(strTools);
}

void Method::removeFileSystemWatch(QString strOpenFile) {
  QString strPath;

  QFileInfo fi(strOpenFile);
  if (!fi.exists()) return;

  strPath = fi.path();
  strACPI = strPath + "/ACPI/";
  strKexts = strPath + "/Kexts/";
  strDrivers = strPath + "/Drivers/";
  strTools = strPath + "/Tools/";

  FileSystemWatcher::removeWatchPath(strACPI);
  FileSystemWatcher::removeWatchPath(strKexts);
  FileSystemWatcher::removeWatchPath(strDrivers);
  FileSystemWatcher::removeWatchPath(strTools);
}

QStringList Method::deDuplication(QStringList FileName, QTableWidget* table,
                                  int col) {
  for (int i = 0; i < table->rowCount(); i++) {
    QString str0 = table->item(i, col)->text().trimmed();
    for (int j = 0; j < FileName.count(); j++) {
      QFileInfo fi(FileName.at(j));
      if (str0 == fi.fileName()) {
        FileName.removeAt(j);
      }
    }
  }
  return FileName;
}

void Method::markColor(QTableWidget* table, QString path, int col) {
  QIcon icon;
  QTableWidgetItem* id1;
  for (int i = 0; i < table->rowCount(); i++) {
    QString strFile = path + table->item(i, col)->text().trimmed();
    QFileInfo fi(strFile);
    if (fi.exists()) {
      icon.addFile(":/icon/green.svg", QSize(10, 10));
      id1 = new QTableWidgetItem(icon, QString::number(i + 1));
      table->setVerticalHeaderItem(i, id1);

      if (table == mw_one->ui->table_kernel_add) {
        QString strVer = getKextVersion(strKexts + table->item(i, 0)->text());
        QString text = table->item(i, 1)->text();
        if (text.trimmed().length() > 0) {
          if (text.mid(0, 1) == "V") {
            QStringList strList = text.split("|");
            if (strList.count() >= 2) {
              text.replace(strList.at(0), "");
              text = "V" + strVer + " " + text;
            } else
              text = "V" + strVer;

          } else
            text = "V" + strVer + " | " + text;

        } else
          text = "V" + strVer;
        table->setItem(i, 1, new QTableWidgetItem(text));
      }

    } else {
      icon.addFile(":/icon/red.svg", QSize(10, 10));
      id1 = new QTableWidgetItem(icon, QString::number(i + 1));
      table->setVerticalHeaderItem(i, id1);
    }
  }
}

void Method::OCValidationProcessing() {
  if (mw_one->ui->chkPickerAudioAssist->isChecked())
    mw_one->ui->chkAudioSupport->setChecked(true);

  //
  for (int i = 0; i < mw_one->ui->table_uefi_drivers->rowCount(); i++) {
    QString str = mw_one->ui->table_uefi_drivers->item(i, 0)->text().trimmed();
    if (str == "Ps2KeyboardDxe.efi")
      mw_one->ui->chkKeySupport->setChecked(true);
  }
}
