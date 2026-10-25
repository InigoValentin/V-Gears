/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
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
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionExit;
    QAction *actionABout;
    QAction *actionHelp;
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
    QLabel *advancedLabelBattle;
    QLabel *advancedLabelGame;
    QLabel *advancedLabelMedia;
    QCheckBox *chk_sounds;
    QCheckBox *chk_music;
    QLabel *advancedLabelFields;
    QLabel *advancedLabelWorld;
    QLabel *advancedLabelOther;
    QCheckBox *chk_clean_install;
    QCheckBox *chk_wm;
    QCheckBox *chk_ffmpeg;
    QCheckBox *chk_images;
    QCheckBox *chk_battle_data;
    QCheckBox *chk_wm_models;
    QCheckBox *chk_timidity;
    QCheckBox *chk_keep_original;
    QCheckBox *chk_fields;
    QCheckBox *chk_kernel;
    QCheckBox *chk_field_models;
    QCheckBox *chk_videos;
    QCheckBox *chk_battle_models;
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
        MainWindow->resize(720, 705);
        MainWindow->setMinimumSize(QSize(0, 0));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionABout = new QAction(MainWindow);
        actionABout->setObjectName(QString::fromUtf8("actionABout"));
        actionHelp = new QAction(MainWindow);
        actionHelp->setObjectName(QString::fromUtf8("actionHelp"));
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
        tab_installer->setMinimumSize(QSize(698, 630));
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
        label_4->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);

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
        isoInfo->setContentsMargins(30, -1, -1, -1);
        isoData = new QLabel(tab_installer);
        isoData->setObjectName(QString::fromUtf8("isoData"));
        isoData->setMargin(0);

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
        label_3->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);

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
        advancedOptions->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(advancedOptions->sizePolicy().hasHeightForWidth());
        advancedOptions->setSizePolicy(sizePolicy);
        advancedOptions->setMinimumSize(QSize(680, 180));
        advancedOptions->setMaximumSize(QSize(16777215, 180));
        advancedOptions->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
        advancedOptions->setFlat(false);
        advancedOptions->setCheckable(false);
        advancedLabelBattle = new QLabel(advancedOptions);
        advancedLabelBattle->setObjectName(QString::fromUtf8("advancedLabelBattle"));
        advancedLabelBattle->setGeometry(QRect(10, 10, 120, 20));
        QFont font;
        font.setBold(true);
        font.setUnderline(true);
        advancedLabelBattle->setFont(font);
        advancedLabelGame = new QLabel(advancedOptions);
        advancedLabelGame->setObjectName(QString::fromUtf8("advancedLabelGame"));
        advancedLabelGame->setGeometry(QRect(10, 30, 120, 20));
        advancedLabelGame->setFont(font);
        advancedLabelMedia = new QLabel(advancedOptions);
        advancedLabelMedia->setObjectName(QString::fromUtf8("advancedLabelMedia"));
        advancedLabelMedia->setGeometry(QRect(10, 50, 120, 20));
        advancedLabelMedia->setFont(font);
        chk_sounds = new QCheckBox(advancedOptions);
        chk_sounds->setObjectName(QString::fromUtf8("chk_sounds"));
        chk_sounds->setGeometry(QRect(360, 50, 200, 20));
        chk_sounds->setMinimumSize(QSize(200, 20));
        chk_sounds->setMaximumSize(QSize(200, 20));
        chk_sounds->setChecked(true);
        chk_music = new QCheckBox(advancedOptions);
        chk_music->setObjectName(QString::fromUtf8("chk_music"));
        chk_music->setGeometry(QRect(150, 70, 200, 20));
        chk_music->setMinimumSize(QSize(200, 20));
        chk_music->setMaximumSize(QSize(200, 20));
        chk_music->setChecked(true);
        advancedLabelFields = new QLabel(advancedOptions);
        advancedLabelFields->setObjectName(QString::fromUtf8("advancedLabelFields"));
        advancedLabelFields->setGeometry(QRect(10, 110, 120, 20));
        advancedLabelFields->setFont(font);
        advancedLabelWorld = new QLabel(advancedOptions);
        advancedLabelWorld->setObjectName(QString::fromUtf8("advancedLabelWorld"));
        advancedLabelWorld->setGeometry(QRect(10, 130, 120, 20));
        advancedLabelWorld->setFont(font);
        advancedLabelOther = new QLabel(advancedOptions);
        advancedLabelOther->setObjectName(QString::fromUtf8("advancedLabelOther"));
        advancedLabelOther->setGeometry(QRect(10, 150, 120, 20));
        advancedLabelOther->setFont(font);
        chk_clean_install = new QCheckBox(advancedOptions);
        chk_clean_install->setObjectName(QString::fromUtf8("chk_clean_install"));
        chk_clean_install->setGeometry(QRect(360, 150, 200, 20));
        chk_clean_install->setMinimumSize(QSize(200, 20));
        chk_clean_install->setMaximumSize(QSize(200, 20));
        chk_clean_install->setCheckable(true);
        chk_wm = new QCheckBox(advancedOptions);
        chk_wm->setObjectName(QString::fromUtf8("chk_wm"));
        chk_wm->setGeometry(QRect(150, 130, 200, 20));
        chk_wm->setMinimumSize(QSize(200, 20));
        chk_wm->setMaximumSize(QSize(200, 20));
        chk_wm->setChecked(true);
        chk_wm->setTristate(false);
        chk_ffmpeg = new QCheckBox(advancedOptions);
        chk_ffmpeg->setObjectName(QString::fromUtf8("chk_ffmpeg"));
        chk_ffmpeg->setGeometry(QRect(150, 90, 200, 20));
        chk_ffmpeg->setMinimumSize(QSize(200, 20));
        chk_ffmpeg->setMaximumSize(QSize(200, 20));
        chk_ffmpeg->setChecked(true);
        chk_images = new QCheckBox(advancedOptions);
        chk_images->setObjectName(QString::fromUtf8("chk_images"));
        chk_images->setGeometry(QRect(150, 50, 200, 20));
        chk_images->setMinimumSize(QSize(200, 20));
        chk_images->setMaximumSize(QSize(200, 20));
        chk_images->setChecked(true);
        chk_battle_data = new QCheckBox(advancedOptions);
        chk_battle_data->setObjectName(QString::fromUtf8("chk_battle_data"));
        chk_battle_data->setGeometry(QRect(150, 10, 200, 20));
        chk_battle_data->setMinimumSize(QSize(200, 20));
        chk_battle_data->setMaximumSize(QSize(200, 20));
        chk_battle_data->setChecked(true);
        chk_wm_models = new QCheckBox(advancedOptions);
        chk_wm_models->setObjectName(QString::fromUtf8("chk_wm_models"));
        chk_wm_models->setGeometry(QRect(360, 130, 200, 20));
        chk_wm_models->setMinimumSize(QSize(200, 20));
        chk_wm_models->setMaximumSize(QSize(200, 20));
        chk_wm_models->setChecked(true);
        chk_timidity = new QCheckBox(advancedOptions);
        chk_timidity->setObjectName(QString::fromUtf8("chk_timidity"));
        chk_timidity->setGeometry(QRect(360, 90, 200, 20));
        chk_timidity->setMinimumSize(QSize(200, 20));
        chk_timidity->setMaximumSize(QSize(200, 20));
        chk_timidity->setChecked(true);
        chk_keep_original = new QCheckBox(advancedOptions);
        chk_keep_original->setObjectName(QString::fromUtf8("chk_keep_original"));
        chk_keep_original->setGeometry(QRect(150, 150, 200, 20));
        chk_keep_original->setMinimumSize(QSize(200, 20));
        chk_keep_original->setMaximumSize(QSize(200, 20));
        chk_keep_original->setChecked(false);
        chk_fields = new QCheckBox(advancedOptions);
        chk_fields->setObjectName(QString::fromUtf8("chk_fields"));
        chk_fields->setGeometry(QRect(150, 110, 200, 20));
        chk_fields->setMinimumSize(QSize(200, 20));
        chk_fields->setMaximumSize(QSize(200, 20));
        chk_fields->setChecked(true);
        chk_kernel = new QCheckBox(advancedOptions);
        chk_kernel->setObjectName(QString::fromUtf8("chk_kernel"));
        chk_kernel->setGeometry(QRect(150, 30, 200, 20));
        chk_kernel->setMinimumSize(QSize(200, 20));
        chk_kernel->setMaximumSize(QSize(200, 20));
        chk_kernel->setChecked(true);
        chk_field_models = new QCheckBox(advancedOptions);
        chk_field_models->setObjectName(QString::fromUtf8("chk_field_models"));
        chk_field_models->setGeometry(QRect(360, 110, 200, 20));
        chk_field_models->setMinimumSize(QSize(200, 20));
        chk_field_models->setMaximumSize(QSize(200, 20));
        chk_field_models->setChecked(true);
        chk_videos = new QCheckBox(advancedOptions);
        chk_videos->setObjectName(QString::fromUtf8("chk_videos"));
        chk_videos->setGeometry(QRect(360, 70, 200, 20));
        chk_videos->setMinimumSize(QSize(200, 20));
        chk_videos->setMaximumSize(QSize(200, 20));
        chk_videos->setChecked(true);
        chk_battle_models = new QCheckBox(advancedOptions);
        chk_battle_models->setObjectName(QString::fromUtf8("chk_battle_models"));
        chk_battle_models->setGeometry(QRect(360, 10, 200, 20));
        chk_battle_models->setMinimumSize(QSize(200, 20));
        chk_battle_models->setMaximumSize(QSize(200, 20));
        chk_battle_models->setBaseSize(QSize(0, 0));
        chk_battle_models->setChecked(true);

        verticalLayout_3->addWidget(advancedOptions);

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
        QFont font1;
        font1.setPointSize(8);
        font1.setItalic(true);
        label_percent->setFont(font1);
        label_percent->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        verticalLayout_3->addWidget(label_percent);

        label_progress = new QLabel(tab_installer);
        label_progress->setObjectName(QString::fromUtf8("label_progress"));
        sizePolicy1.setHeightForWidth(label_progress->sizePolicy().hasHeightForWidth());
        label_progress->setSizePolicy(sizePolicy1);
        label_progress->setMinimumSize(QSize(0, 0));
        label_progress->setMaximumSize(QSize(16777215, 10));
        label_progress->setFont(font1);
        label_progress->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        verticalLayout_3->addWidget(label_progress);

        label_5 = new QLabel(tab_installer);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMaximumSize(QSize(16777215, 15));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Courier"));
        font2.setPointSize(8);
        label_5->setFont(font2);
        label_5->setMargin(-1);

        verticalLayout_3->addWidget(label_5);

        data_log = new QTextEdit(tab_installer);
        data_log->setObjectName(QString::fromUtf8("data_log"));
        data_log->setFont(font2);
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
        QWidget::setTabOrder(tabWidget, line_data_src);
        QWidget::setTabOrder(line_data_src, btn_data_src);
        QWidget::setTabOrder(btn_data_src, help_data_src);
        QWidget::setTabOrder(help_data_src, line_data_dst);
        QWidget::setTabOrder(line_data_dst, btn_data_dst);
        QWidget::setTabOrder(btn_data_dst, help_data_dest);
        QWidget::setTabOrder(help_data_dest, chk_advanced_options);
        QWidget::setTabOrder(chk_advanced_options, chk_battle_data);
        QWidget::setTabOrder(chk_battle_data, chk_battle_models);
        QWidget::setTabOrder(chk_battle_models, chk_kernel);
        QWidget::setTabOrder(chk_kernel, chk_images);
        QWidget::setTabOrder(chk_images, chk_sounds);
        QWidget::setTabOrder(chk_sounds, chk_music);
        QWidget::setTabOrder(chk_music, chk_videos);
        QWidget::setTabOrder(chk_videos, chk_ffmpeg);
        QWidget::setTabOrder(chk_ffmpeg, chk_timidity);
        QWidget::setTabOrder(chk_timidity, chk_fields);
        QWidget::setTabOrder(chk_fields, chk_field_models);
        QWidget::setTabOrder(chk_field_models, chk_wm);
        QWidget::setTabOrder(chk_wm, chk_wm_models);
        QWidget::setTabOrder(chk_wm_models, chk_keep_original);
        QWidget::setTabOrder(chk_keep_original, chk_clean_install);
        QWidget::setTabOrder(chk_clean_install, btn_data_run);
        QWidget::setTabOrder(btn_data_run, data_log);
        QWidget::setTabOrder(data_log, line_vgears_exe);
        QWidget::setTabOrder(line_vgears_exe, btn_vgears_exe);
        QWidget::setTabOrder(btn_vgears_exe, line_vgears_config);
        QWidget::setTabOrder(line_vgears_config, btn_vgears_config);
        QWidget::setTabOrder(btn_vgears_config, btn_vgears_launch);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionABout);
        menuFile->addAction(actionHelp);
        menuFile->addSeparator();
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
        actionABout->setText(QCoreApplication::translate("MainWindow", "About", nullptr));
        actionHelp->setText(QCoreApplication::translate("MainWindow", "Help", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Final Fantasy VII ISO file: ", nullptr));
        btn_data_src->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        help_data_src->setToolTip(QCoreApplication::translate("MainWindow", "\n"
"              <html><head/><body><b>Path to the extracted FFVII (PC version) data.</b><br><br>It must contain (at least) the folders 'kernel' and 'field'.</body></html>\n"
"             ", nullptr));
#endif // QT_CONFIG(tooltip)
        help_data_src->setText(QString());
        isoData->setText(QString());
        isoError->setText(QString());
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
        advancedLabelBattle->setText(QCoreApplication::translate("MainWindow", "Battle data", nullptr));
        advancedLabelGame->setText(QCoreApplication::translate("MainWindow", "Game data", nullptr));
        advancedLabelMedia->setText(QCoreApplication::translate("MainWindow", "Multimedia", nullptr));
#if QT_CONFIG(tooltip)
        chk_sounds->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">Don't extract sound.</span><br/><br/>If checked, game sound efects will not be extracted.<br/><br/>Sound extraction can take a few minutes, and it's ok to skip this step if it has already been installed.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_sounds->setText(QCoreApplication::translate("MainWindow", "Don't extract sounds", nullptr));
#if QT_CONFIG(tooltip)
        chk_music->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">Don't extract music.</span><br/><br/>If checked, game music tracks will not be extracted.<br/><br/>Music extraction can take a few minutes, and it's ok to skip this step if it has already been installed.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_music->setText(QCoreApplication::translate("MainWindow", "Don't extract music", nullptr));
        advancedLabelFields->setText(QCoreApplication::translate("MainWindow", "Field Maps", nullptr));
        advancedLabelWorld->setText(QCoreApplication::translate("MainWindow", "World Map", nullptr));
        advancedLabelOther->setText(QCoreApplication::translate("MainWindow", "Other", nullptr));
#if QT_CONFIG(tooltip)
        chk_clean_install->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">Delete previously installed files.</span></p><p><br/></p><p>Performs a clean installation, unconditionally removing all previously installed data.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_clean_install->setText(QCoreApplication::translate("MainWindow", "Clean install", nullptr));
#if QT_CONFIG(tooltip)
        chk_wm->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract world map data.</b><br><br>If checked, world map models and data will not be extracted. Data from previous installations will not be deleted.<br><br>This installation step is not usually very long, so there is no reason to skip it unless you have manually edited the world map data. Skipping world map data installation will also skip the installation of the world map 3D models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_wm->setText(QCoreApplication::translate("MainWindow", "Don't extract world map data", nullptr));
#if QT_CONFIG(tooltip)
        chk_ffmpeg->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't use the ffmpeg command.</b><br><br>If checked, sound effects and background music will not be available to V-Gears.<br><br>Sound effects and music indexes will still be built, but you will need to provide your own ogg files.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_ffmpeg->setText(QCoreApplication::translate("MainWindow", "Prevent FFmpeg calls", nullptr));
#if QT_CONFIG(tooltip)
        chk_images->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract images.</b><br><br>If checked, images will not be extracted. This includes window decorations, menus icons, character portraits... It doesn' include backgrounds or textures. Images from previous installations will not be deleted.<br><br>This installation step is usually quite fast, so there is no reason to skip it unless you have manually edited the images.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_images->setText(QCoreApplication::translate("MainWindow", "Don't extract images", nullptr));
#if QT_CONFIG(tooltip)
        chk_battle_data->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract battle data.</b><br><br>If checked, battle data such as enemies, attacks or enemy formations will not be extracted. Data from previous installations will not be deleted.<br><br>This installation step is usually quite fast, so there is no reason to skip it unless you have manually changed data.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_battle_data->setText(QCoreApplication::translate("MainWindow", "Don't extract battle data", nullptr));
#if QT_CONFIG(tooltip)
        chk_wm_models->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract world map 3D models.</b><br><br>If checked, world map 3D models and textures will not be extracted. Models from previous installations will not be deleted.<br><br>This installation step is not usually very long, so there is no reason to skip it unless you have manually edited the models. Skipping world map data installation will also skip the installation of world map 3D models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_wm_models->setText(QCoreApplication::translate("MainWindow", "Dont't extract world 3D models", nullptr));
#if QT_CONFIG(tooltip)
        chk_timidity->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't use the timidity command.</b><br><br>If checked, background music will not be available to V-Gears.<br><br>Music indexes will still be built, but you will need to provide your own ogg tracks.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_timidity->setText(QCoreApplication::translate("MainWindow", "Prevent TiMidity calls", nullptr));
#if QT_CONFIG(tooltip)
        chk_keep_original->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't delete original data after installation.</b><br><br>The installer extracts some original data from the installation disk, such as MIDI sounds, TEX images, and some LGP archives. If checked, this data will not be deleted when the installation is complete.<br><br>This data is never used by V-Gears, and there is usually no need to check this.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_keep_original->setText(QCoreApplication::translate("MainWindow", "Preserve original data", nullptr));
#if QT_CONFIG(tooltip)
        chk_fields->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract field maps.</b><br><br>If checked, map fields will not be extracted. Field maps from previous installations will not be deleted.<br><br>This installation step is not usually very long, so there is no reason to skip it unless you have manually edited the fields. Skipping field map installation will also skip the installation of field 3D models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_fields->setText(QCoreApplication::translate("MainWindow", "Don't extract field maps", nullptr));
#if QT_CONFIG(tooltip)
        chk_kernel->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract kernel data.</b><br><br>If checked, game data such as items, materia, character information or initial savemap will not be extracted. Data from previous installations will not be deleted.<br><br>This installation step is usually quite fast, so there is no reason to skip it unless you have manually changed data.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_kernel->setText(QCoreApplication::translate("MainWindow", "Don't extract kernel data", nullptr));
#if QT_CONFIG(tooltip)
        chk_field_models->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract field 3D models.</b><br><br>If checked, field 3D models and textures will not be extracted. Models from previous installations will not be deleted.<br><br>This installation step is not usually very long, so there is no reason to skip it unless you have manually edited the models. Skipping field map installation will also skip the installation of field 3D models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_field_models->setText(QCoreApplication::translate("MainWindow", "Dont't extract field 3D models", nullptr));
#if QT_CONFIG(tooltip)
        chk_videos->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract videos.</b><br><br>If checked, FMV videos will not be extracted. Videos from previous installations will not be deleted.<br><br>Installing videos can take a few minutes, and it's OK to skip this step if they are already installed.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_videos->setText(QCoreApplication::translate("MainWindow", "Don't extract videos", nullptr));
#if QT_CONFIG(tooltip)
        chk_battle_models->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><b>Don't extract battle models.</b><br><br>If checked, battle 3D models will not be extracted. This includes models for players, enemies, attacks, battle scenarios... Models from previous installations will not be deleted.<br><br>This installation step is usually quite fast, so there is no reason to skip it unless you have manually edited the models.</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        chk_battle_models->setText(QCoreApplication::translate("MainWindow", "Don't extract battle models", nullptr));
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
