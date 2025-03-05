#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar> 
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QColorDialog>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QIcon>
#include <QPixmap>

#include <QInputDialog> // Ajout pour le dialogue de saisie
#include "../terminalwidget.h" // Note: Assurez-vous d'avoir cette bibliothèque

#include <signal.h>

class TerminalWindow : public QMainWindow {
    Q_OBJECT

public:
    TerminalWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setupUi();
        loadConfig();
    }

    ~TerminalWindow() {
        saveConfig();
    }

private:
    QTabWidget *tabWidget;
    QString configFile = "terminal_config.json";


    void setupUi() {
        // Création du TabWidget
        tabWidget = new QTabWidget(this);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        setCentralWidget(tabWidget);

        // Connexion pour fermer les onglets
        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &TerminalWindow::closeTab);

        // Création de la barre de menu
        QMenuBar *menuBar = new QMenuBar(this);
        setMenuBar(menuBar);

        // Menu Fichier
        QMenu *fileMenu = menuBar->addMenu(tr("&Fichier"));
        QAction *newTabAction = fileMenu->addAction(tr("Nouvel onglet"));
        QAction *saveAction = fileMenu->addAction(tr("&Sauvegarder")); // Nouvelle action
        QAction *loadAction = fileMenu->addAction(tr("&Charger conf.")); // Nouvelle action
        QAction *quitAction = fileMenu->addAction(tr("&Quitter"));

        // Menu Aide
        QMenu *helpMenu = menuBar->addMenu(tr("&Aide"));
        QAction *aboutAction = helpMenu->addAction(tr("À propos"));

        // Toolbar
        QToolBar *toolBar = addToolBar(tr("Outils"));
        toolBar->addAction(newTabAction);
        toolBar->addAction(quitAction);
        toolBar->addAction(aboutAction);

        // Connexions des actions
        connect(newTabAction, &QAction::triggered, this, &TerminalWindow::addNewTab);
        connect(quitAction, &QAction::triggered, this, &QApplication::quit);
        connect(aboutAction, &QAction::triggered, this, &TerminalWindow::showAbout);

        // Menu contextuel pour les onglets
        tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tabWidget, &QTabWidget::customContextMenuRequested, 
                this, &TerminalWindow::showContextMenu);

        // Taille initiale
        resize(800, 600);
    }

    void addNewTab() {
        TerminalWidget *terminal = new TerminalWidget;
        int tabIndex = tabWidget->addTab(terminal, QString("Terminal %1").arg(tabWidget->count() + 1));

        tabWidget->setCurrentIndex(tabIndex);
    }

   void setColorIconTab(QColor color,int tabIndex) {
                QPixmap pixmap(100,100);
                pixmap.fill(color);
                QIcon colorIcon(pixmap);

                tabWidget->tabBar()->setTabIcon(tabIndex, colorIcon);
                tabWidget->tabBar()->tabButton(tabIndex,QTabBar::RightSide)->setStyleSheet(
                    QString("background-color: %1").arg(color.name()));
                tabWidget->tabBar()->setTabTextColor(tabIndex, color);
                /*tabWidget->widget(tabIndex)->setStyleSheet(
                    QString("background-color: %1").arg(color.name()));*/
   } 

private slots:
    void closeTab(int index) {
        TerminalWidget *t =( TerminalWidget *)  tabWidget->widget(index);
        if (t != 0x0) 
        {
          kill(t->shellPid(),9);
        }
        tabWidget->removeTab(index);
    }

    void showContextMenu(const QPoint &pos) {

        int tabIndex = tabWidget->tabBar()->tabAt(pos);

        if (tabIndex == -1) return;

        QMenu contextMenu(this);
        QAction *colorAction = contextMenu.addAction("Changer la couleur");
        QAction *renameAction = contextMenu.addAction("Renommer l'onglet"); // Nouvelle action

        
        connect(colorAction, &QAction::triggered, this, [this, tabIndex]() {
            QColor color = QColorDialog::getColor(Qt::white, this);
            if (color.isValid()) {

		setColorIconTab(color,tabIndex);               
            }
        });


        // Action pour renommer l'onglet
        connect(renameAction, &QAction::triggered, this, [this, tabIndex]() {
            bool ok;
            QString newName = QInputDialog::getText(this, "Renommer l'onglet",
                                                  "Entrez le nouveau nom:", 
                                                  QLineEdit::Normal,
                                                  tabWidget->tabText(tabIndex), &ok);
            if (ok && !newName.isEmpty()) {
                tabWidget->setTabText(tabIndex, newName);
            }
        });

        contextMenu.exec(tabWidget->mapToGlobal(pos));
    }

    void showAbout() {
        QMessageBox::about(this, "À propos", 
                          "Application Terminal\nVersion 1.0\nCréée avec Qt");
    }

    void saveConfig() {
        QJsonObject config;
        QJsonArray tabsArray;

        for (int i = 0; i < tabWidget->count(); i++) {
            QJsonObject tabObj;
            tabObj["title"] = tabWidget->tabText(i);
            TerminalWidget *t =( TerminalWidget *)  tabWidget->widget(i);
            tabObj["path"] = t->workingDirectory();

            QColor c = tabWidget->tabBar()->tabTextColor(i);
            tabObj["color"] = c.name();
            tabsArray.append(tabObj);
        }

        config["tabs"] = tabsArray;

        QFile file(configFile);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(config).toJson());
            file.close();
        }
    }

    void loadConfig() {
        QFile file(configFile);
        if (!file.open(QIODevice::ReadOnly)) return;

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject config = doc.object();
        QJsonArray tabsArray = config["tabs"].toArray();

        for (const QJsonValue &value : tabsArray) {
            QJsonObject tabObj = value.toObject();
            TerminalWidget *terminal = new TerminalWidget;
            int index = tabWidget->addTab(terminal, tabObj["title"].toString());

            if (tabObj.contains("path")) {
                 terminal->changeDir(tabObj["path"].toString());
                 terminal->clear();
            }
            
            if (tabObj.contains("color")) {
		        setColorIconTab(QColor(tabObj["color"].toString()),index);
            }
        }
        file.close();
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    TerminalWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
