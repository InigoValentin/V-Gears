/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.17
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionExit;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tab_installer;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *isoLocator;
    QLabel *label_4;
    QLineEdit *line_data_src;
    QPushButton *btn_data_src;
    QPushButton *help_data_src;
    QVBoxLayout *isoInfo;
    QLabel *isoData;
    QLabel *isoError;
    QHBoxLayout *output;
    QLabel *label_3;
    QLineEdit *line_data_dst;
    QPushButton *btn_data_dst;
    QPushButton *help_data_dest;
    QCheckBox *chk_advanced_options;
    QGroupBox *advancedOptions;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_6;
    QCheckBox *chk_no_battle_data;
    QCheckBox *chk_no_battle_models;
    QHBoxLayout *horizontalLayout_61;
    QCheckBox *chk_no_kernel;
    QCheckBox *chk_no_images;
    QHBoxLayout *horizontalLayout_7;
    QCheckBox *chk_no_sounds;
    QCheckBox *chk_no_music;
    QHBoxLayout *horizontalLayout_8;
    QCheckBox *chk_no_fields;
    QCheckBox *chk_no_field_models;
    QHBoxLayout *horizontalLayout_9;
    QCheckBox *chk_no_wm;
    QCheckBox *chk_no_wm_models;
    QHBoxLayout *horizontalLayout_5;
    QCheckBox *chk_no_ffmpeg;
    QCheckBox *chk_no_timidity;
    QHBoxLayout *horizontalLayout_91;
    QCheckBox *chk_keep_original;
    QSpacerItem *verticalSpacer;
    QPushButton *btn_data_run;
    QProgressBar *data_progress_bar;
    QLabel *label_percent;
    QLabel *label_progress;
    QLabel *label_5;
    QTextEdit *data_log;
    QWidget *tab_vgears;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *line_vgears_exe;
    QPushButton *btn_vgears_exe;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLineEdit *line_vgears_config;
    QPushButton *btn_vgears_config;
    QPushButton *btn_vgears_launch;
    QMenuBar *menuBar;
    QMenu *menuFile;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(720, 703);
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setTabPosition(QTabWidget::TabPosition::North);
        tab_installer = new QWidget();
        tab_installer->setObjectName(QString::fromUtf8("tab_installer"));
        verticalLayout_3 = new QVBoxLayout(tab_installer);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        isoLocator = new QHBoxLayout();
        isoLocator->setSpacing(6);
        isoLocator->setObjectName(QString::fromUtf8("isoLocator"));
        label_4 = new QLabel(tab_installer);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setMinimumSize(QSize(200, 0));
        label_4->setMaximumSize(QSize(200, 50));
        label_4->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        isoLocator->addWidget(label_4);

        line_data_src = new QLineEdit(tab_installer);
        line_data_src->setObjectName(QString::fromUtf8("line_data_src"));

        isoLocator->addWidget(line_data_src);

        btn_data_src = new QPushButton(tab_installer);
        btn_data_src->setObjectName(QString::fromUtf8("btn_data_src"));
        btn_data_src->setMaximumSize(QSize(24, 24));

        isoLocator->addWidget(btn_data_src);

        help_data_src = new QPushButton(tab_installer);
        help_data_src->setObjectName(QString::fromUtf8("help_data_src"));
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("dialog-information");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8("Workspace/Software/V-Gears/V-Gears-Installer/src"), QSize(), QIcon::Normal, QIcon::Off);
        }
        help_data_src->setIcon(icon);
        help_data_src->setCheckable(false);
        help_data_src->setFlat(true);

        isoLocator->addWidget(help_data_src);


        verticalLayout_3->addLayout(isoLocator);

        isoInfo = new QVBoxLayout();
        isoInfo->setSpacing(6);
        isoInfo->setObjectName(QString::fromUtf8("isoInfo"));
        isoData = new QLabel(tab_installer);
        isoData->setObjectName(QString::fromUtf8("isoData"));

        isoInfo->addWidget(isoData);

        isoError = new QLabel(tab_installer);
        isoError->setObjectName(QString::fromUtf8("isoError"));

        isoInfo->addWidget(isoError);


        verticalLayout_3->addLayout(isoInfo);

        output = new QHBoxLayout();
        output->setSpacing(6);
        output->setObjectName(QString::fromUtf8("output"));
        label_3 = new QLabel(tab_installer);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(200, 0));
        label_3->setMaximumSize(QSize(200, 50));
        label_3->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        output->addWidget(label_3);

        line_data_dst = new QLineEdit(tab_installer);
        line_data_dst->setObjectName(QString::fromUtf8("line_data_dst"));

        output->addWidget(line_data_dst);

        btn_data_dst = new QPushButton(tab_installer);
        btn_data_dst->setObjectName(QString::fromUtf8("btn_data_dst"));
        btn_data_dst->setMaximumSize(QSize(24, 24));

        output->addWidget(btn_data_dst);

        help_data_dest = new QPushButton(tab_installer);
        help_data_dest->setObjectName(QString::fromUtf8("help_data_dest"));
        help_data_dest->setIcon(icon);
        help_data_dest->setCheckable(false);
        help_data_dest->setFlat(true);

        output->addWidget(help_data_dest);


        verticalLayout_3->addLayout(output);

        chk_advanced_options = new QCheckBox(tab_installer);
        chk_advanced_options->setObjectName(QString::fromUtf8("chk_advanced_options"));

        verticalLayout_3->addWidget(chk_advanced_options);

        advancedOptions = new QGroupBox(tab_installer);
        advancedOptions->setObjectName(QString::fromUtf8("advancedOptions"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(advancedOptions->sizePolicy().hasHeightForWidth());
        advancedOptions->setSizePolicy(sizePolicy);
        advancedOptions->setMaximumSize(QSize(16777215, 0));
        verticalLayout_5 = new QVBoxLayout(advancedOptions);
        verticalLayout_5->setSpacing(5);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(11, -1, -1, -1);
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        chk_no_battle_data = new QCheckBox(advancedOptions);
        chk_no_battle_data->setObjectName(QString::fromUtf8("chk_no_battle_data"));

        horizontalLayout_6->addWidget(chk_no_battle_data);

        chk_no_battle_models = new QCheckBox(advancedOptions);
        chk_no_battle_models->setObjectName(QString::fromUtf8("chk_no_battle_models"));

        horizontalLayout_6->addWidget(chk_no_battle_models);


        verticalLayout_5->addLayout(horizontalLayout_6);

        horizontalLayout_61 = new QHBoxLayout();
        horizontalLayout_61->setSpacing(6);
        horizontalLayout_61->setObjectName(QString::fromUtf8("horizontalLayout_61"));
        chk_no_kernel = new QCheckBox(advancedOptions);
        chk_no_kernel->setObjectName(QString::fromUtf8("chk_no_kernel"));

        horizontalLayout_61->addWidget(chk_no_kernel);

        chk_no_images = new QCheckBox(advancedOptions);
        chk_no_images->setObjectName(QString::fromUtf8("chk_no_images"));

        horizontalLayout_61->addWidget(chk_no_images);


        verticalLayout_5->addLayout(horizontalLayout_61);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        chk_no_sounds = new QCheckBox(advancedOptions);
        chk_no_sounds->setObjectName(QString::fromUtf8("chk_no_sounds"));

        horizontalLayout_7->addWidget(chk_no_sounds);

        chk_no_music = new QCheckBox(advancedOptions);
        chk_no_music->setObjectName(QString::fromUtf8("chk_no_music"));

        horizontalLayout_7->addWidget(chk_no_music);


        verticalLayout_5->addLayout(horizontalLayout_7);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        chk_no_fields = new QCheckBox(advancedOptions);
        chk_no_fields->setObjectName(QString::fromUtf8("chk_no_fields"));

        horizontalLayout_8->addWidget(chk_no_fields);

        chk_no_field_models = new QCheckBox(advancedOptions);
        chk_no_field_models->setObjectName(QString::fromUtf8("chk_no_field_models"));

        horizontalLayout_8->addWidget(chk_no_field_models);


        verticalLayout_5->addLayout(horizontalLayout_8);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        chk_no_wm = new QCheckBox(advancedOptions);
        chk_no_wm->setObjectName(QString::fromUtf8("chk_no_wm"));

        horizontalLayout_9->addWidget(chk_no_wm);

        chk_no_wm_models = new QCheckBox(advancedOptions);
        chk_no_wm_models->setObjectName(QString::fromUtf8("chk_no_wm_models"));

        horizontalLayout_9->addWidget(chk_no_wm_models);


        verticalLayout_5->addLayout(horizontalLayout_9);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        chk_no_ffmpeg = new QCheckBox(advancedOptions);
        chk_no_ffmpeg->setObjectName(QString::fromUtf8("chk_no_ffmpeg"));

        horizontalLayout_5->addWidget(chk_no_ffmpeg);

        chk_no_timidity = new QCheckBox(advancedOptions);
        chk_no_timidity->setObjectName(QString::fromUtf8("chk_no_timidity"));

        horizontalLayout_5->addWidget(chk_no_timidity);


        verticalLayout_5->addLayout(horizontalLayout_5);

        horizontalLayout_91 = new QHBoxLayout();
        horizontalLayout_91->setSpacing(6);
        horizontalLayout_91->setObjectName(QString::fromUtf8("horizontalLayout_91"));
        chk_keep_original = new QCheckBox(advancedOptions);
        chk_keep_original->setObjectName(QString::fromUtf8("chk_keep_original"));

        horizontalLayout_91->addWidget(chk_keep_original);


        verticalLayout_5->addLayout(horizontalLayout_91);


        verticalLayout_3->addWidget(advancedOptions);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Fixed, QSizePolicy::Minimum);

        verticalLayout_3->addItem(verticalSpacer);

        btn_data_run = new QPushButton(tab_installer);
        btn_data_run->setObjectName(QString::fromUtf8("btn_data_run"));

        verticalLayout_3->addWidget(btn_data_run);

        data_progress_bar = new QProgressBar(tab_installer);
        data_progress_bar->setObjectName(QString::fromUtf8("data_progress_bar"));
        data_progress_bar->setValue(0);
        data_progress_bar->setAlignment(Qt::AlignmentFlag::AlignCenter);
        data_progress_bar->setTextVisible(false);

        verticalLayout_3->addWidget(data_progress_bar);

        label_percent = new QLabel(tab_installer);
        label_percent->setObjectName(QString::fromUtf8("label_percent"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_percent->sizePolicy().hasHeightForWidth());
        label_percent->setSizePolicy(sizePolicy1);
        label_percent->setMinimumSize(QSize(0, 0));
        label_percent->setMaximumSize(QSize(16777215, 10));
        QFont font;
        font.setPointSize(8);
        font.setItalic(true);
        label_percent->setFont(font);
        label_percent->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        verticalLayout_3->addWidget(label_percent);

        label_progress = new QLabel(tab_installer);
        label_progress->setObjectName(QString::fromUtf8("label_progress"));
        sizePolicy1.setHeightForWidth(label_progress->sizePolicy().hasHeightForWidth());
        label_progress->setSizePolicy(sizePolicy1);
        label_progress->setMinimumSize(QSize(0, 0));
        label_progress->setMaximumSize(QSize(16777215, 10));
        label_progress->setFont(font);
        label_progress->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        verticalLayout_3->addWidget(label_progress);

        label_5 = new QLabel(tab_installer);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMaximumSize(QSize(16777215, 15));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Courier"));
        font1.setPointSize(8);
        label_5->setFont(font1);
        label_5->setMargin(-1);

        verticalLayout_3->addWidget(label_5);

        data_log = new QTextEdit(tab_installer);
        data_log->setObjectName(QString::fromUtf8("data_log"));
        data_log->setFont(font1);
        data_log->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        data_log->setUndoRedoEnabled(false);
        data_log->setReadOnly(true);

        verticalLayout_3->addWidget(data_log);

        tabWidget->addTab(tab_installer, QString());
        tab_vgears = new QWidget();
        tab_vgears->setObjectName(QString::fromUtf8("tab_vgears"));
        verticalLayout_2 = new QVBoxLayout(tab_vgears);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(tab_vgears);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        line_vgears_exe = new QLineEdit(tab_vgears);
        line_vgears_exe->setObjectName(QString::fromUtf8("line_vgears_exe"));

        horizontalLayout->addWidget(line_vgears_exe);

        btn_vgears_exe = new QPushButton(tab_vgears);
        btn_vgears_exe->setObjectName(QString::fromUtf8("btn_vgears_exe"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(btn_vgears_exe->sizePolicy().hasHeightForWidth());
        btn_vgears_exe->setSizePolicy(sizePolicy2);
        btn_vgears_exe->setMaximumSize(QSize(24, 24));

        horizontalLayout->addWidget(btn_vgears_exe);


        verticalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(tab_vgears);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        line_vgears_config = new QLineEdit(tab_vgears);
        line_vgears_config->setObjectName(QString::fromUtf8("line_vgears_config"));

        horizontalLayout_2->addWidget(line_vgears_config);

        btn_vgears_config = new QPushButton(tab_vgears);
        btn_vgears_config->setObjectName(QString::fromUtf8("btn_vgears_config"));
        sizePolicy2.setHeightForWidth(btn_vgears_config->sizePolicy().hasHeightForWidth());
        btn_vgears_config->setSizePolicy(sizePolicy2);
        btn_vgears_config->setMaximumSize(QSize(24, 24));

        horizontalLayout_2->addWidget(btn_vgears_config);


        verticalLayout_2->addLayout(horizontalLayout_2);

        btn_vgears_launch = new QPushButton(tab_vgears);
        btn_vgears_launch->setObjectName(QString::fromUtf8("btn_vgears_launch"));

        verticalLayout_2->addWidget(btn_vgears_launch);

        tabWidget->addTab(tab_vgears, QString());

        verticalLayout->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 720, 23));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionExit);

        retranslateUi(MainWindow);
        QObject::connect(actionExit, SIGNAL(triggered()), MainWindow, SLOT(close()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "VGears Installer", nullptr));
        actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Original FFVII extracted data:", nullptr));
        btn_data_src->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        help_data_src->setToolTip(QCoreApplication::translate("MainWindow", "\n"
"              <html><head/><body><b>Path to the extracted FFVII (PC version) data.</b><br><br>It must contain (at least) the folders 'kernel' and 'field'.</body></html>\n"
"             ", nullptr));
#endif // QT_CONFIG(tooltip)
        help_data_src->setText(QString());
        isoData->setText(QCoreApplication::translate("MainWindow", "ISO Info: ", nullptr));
        isoError->setText(QCoreApplication::translate("MainWindow", "Message", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "VGears installation directory:", nullptr));
        btn_data_dst->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        help_data_dest->setToolTip(QCoreApplication::translate("MainWindow", "\n"
"              <html><head/><body><b>V-Gears data installation directory.</b><br><br>The data will be installed to this location. Existing installations will be overwritten.</body></html>\n"
"             ", nullptr));
#endif // QT_CONFIG(tooltip)
        help_data_dest->setText(QString());
        chk_advanced_options->setText(QCoreApplication::translate("MainWindow", "Show advanced options", nullptr));
        advancedOptions->setTitle(QString());
#if QT_CONFIG(tooltip)
        chk_no_battle_data->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract battle data.</b><br><br>If checked, battle data such as enemies, attacks or enemy formations will not be extracted. Data from previous installations will not be deleted.<br><br>This installation step is usually quite fast, so there is no reason to skip it unless you have manually changed data.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_battle_data->setText(QCoreApplication::translate("MainWindow", "Don't extract battle data", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_battle_models->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract battle models.</b><br><br>If checked, battle 3D models will not be extracted. This includes models for players, enemies, attacks, battle scenarios... Models from previous installations will not be deleted.<br><br>This installation step is usually quite fast, so there is no reason to skip it unless you have manually edited the models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_battle_models->setText(QCoreApplication::translate("MainWindow", "Don't extract battle models", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_kernel->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract kernel data.</b><br><br>If checked, game data such as items, materia, character information or initial savemap will not be extracted. Data from previous installations will not be deleted.<br><br>This installation step is usually quite fast, so there is no reason to skip it unless you have manually changed data.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_kernel->setText(QCoreApplication::translate("MainWindow", "Don't extract kernel data", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_images->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract images.</b><br><br>If checked, images will not be extracted. This includes window decorations, menus icons, character portraits... It doesn' include backgrounds or textures. Images from previous installations will not be deleted.<br><br>This installation step is usually quite fast, so there is no reason to skip it unless you have manually edited the images.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_images->setText(QCoreApplication::translate("MainWindow", "Don't extract images", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_sounds->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract sounds.</b><br><br>If checked, sound effects will not be extracted. This doesn' include music tracks. Sounds from previous installations will not be deleted.<br><br>Installing sounds can take a few minutes, and it's OK to skip this step if they are already installed. You can also check this if you are getting errors installing sounds or if FFmpeg is not installed in your system.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_sounds->setText(QCoreApplication::translate("MainWindow", "Don't extract sounds", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_music->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract audio tracks.</b><br><br>If checked, audio tracks will not be extracted. This doesn' include sound effects. Tracks from previous installations will not be deleted.<br><br>Installing music can take a few minutes, and it's OK to skip this step if they are already installed. You can also check this if you are getting errors installing sounds or if FFmpeg or TiMidity are not installed in your system.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_music->setText(QCoreApplication::translate("MainWindow", "Don't extract music tracks", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_fields->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract field maps.</b><br><br>If checked, map fields will not be extracted. Field maps from previous installations will not be deleted.<br><br>This installation step is not usually very long, so there is no reason to skip it unless you have manually edited the fields. Skipping field map installation will also skip the installation of field 3D models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_fields->setText(QCoreApplication::translate("MainWindow", "Don't extract field maps", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_field_models->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract field 3D models.</b><br><br>If checked, field 3D models and textures will not be extracted. Models from previous installations will not be deleted.<br><br>This installation step is not usually very long, so there is no reason to skip it unless you have manually edited the models. Skipping field map installation will also skip the installation of field 3D models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_field_models->setText(QCoreApplication::translate("MainWindow", "Dont't extract field 3D models", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_wm->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract world map data.</b><br><br>If checked, world map models and data will not be extracted. Data from previous installations will not be deleted.<br><br>This installation step is not usually very long, so there is no reason to skip it unless you have manually edited the world map data. Skipping world map data installation will also skip the installation of the world map 3D models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_wm->setText(QCoreApplication::translate("MainWindow", "Don't extract world map data", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_wm_models->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract world map 3D models.</b><br><br>If checked, world map 3D models and textures will not be extracted. Models from previous installations will not be deleted.<br><br>This installation step is not usually very long, so there is no reason to skip it unless you have manually edited the models. Skipping world map data installation will also skip the installation of world map 3D models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_wm_models->setText(QCoreApplication::translate("MainWindow", "Dont't extract world 3D models", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_ffmpeg->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't use the ffmpeg command.</b><br><br>If checked, sound effects and background music will not be available to V-Gears.<br><br>Sound effects and music indexes will still be built, but you will need to provide your own ogg files.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_ffmpeg->setText(QCoreApplication::translate("MainWindow", "Prevent FFmpeg calls", nullptr));
#if QT_CONFIG(tooltip)
        chk_no_timidity->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't use the timidity command.</b><br><br>If checked, background music will not be available to V-Gears.<br><br>Music indexes will still be built, but you will need to provide your own ogg tracks.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_no_timidity->setText(QCoreApplication::translate("MainWindow", "Prevent TiMidity calls", nullptr));
#if QT_CONFIG(tooltip)
        chk_keep_original->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't delete original data after installation.</b><br><br>The installer extracts some original data from the installation disk, such as MIDI sounds, TEX images, and some LGP archives. If checked, this data will not be deleted when the installation is complete.<br><br>This data is never used by V-Gears, and there is usually no need to check this.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_keep_original->setText(QCoreApplication::translate("MainWindow", "Preserve original data", nullptr));
        btn_data_run->setText(QCoreApplication::translate("MainWindow", "Install data", nullptr));
        label_percent->setText(QString());
        label_progress->setText(QString());
        label_5->setText(QCoreApplication::translate("MainWindow", "Installer log:", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_installer), QCoreApplication::translate("MainWindow", "Data Installer", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "VGears Path", nullptr));
        btn_vgears_exe->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Config Directory", nullptr));
        btn_vgears_config->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        btn_vgears_launch->setText(QCoreApplication::translate("MainWindow", "Run V-Gears", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_vgears), QCoreApplication::translate("MainWindow", "V-Gears Config", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
