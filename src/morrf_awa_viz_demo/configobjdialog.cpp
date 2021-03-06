#include "configobjdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>

#define WEIGHTED_SUM_STR          "Weighted-sum"
#define TCHEBYCHEFF_STR           "Tchebycheff"
#define BOUNDARY_INTERSECTION_STR "Boundary-intersection"

ConfigObjDialog::ConfigObjDialog(MainWindow * parent) {
    mpParentWindow = parent;

    mpCheckMinDist = new QCheckBox();
    if (mpParentWindow->mpViz->mMOPPInfo.mMinDistEnabled==true) {
        mpCheckMinDist->setChecked(true);
    }
    else {
        mpCheckMinDist->setChecked(false);
    }
    //connect(mpCheckMinDist , SIGNAL(stateChanged(int)),this,SLOT(checkBoxStateChanged(int)));
    mpLabelMinDist = new QLabel("Minimize distance");
    mpLabelSubProb = new QLabel("Subproblem Num: ");
    mpLineEditSubProb = new QLineEdit();
    mpLineEditSubProb->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mSubproblemNum));
    mpLineEditSubProb->setMaximumWidth(40);
    mpLabelIterationNum = new QLabel("Iteration Num: ");
    mpLineEditIterationNum = new QLineEdit();
    mpLineEditIterationNum->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mMaxIterationNum));
    mpLineEditIterationNum->setMaximumWidth(80);
    mpLabelSegmentLength = new QLabel("Segment Len: ");
    mpLineEditSegmentLength = new QLineEdit();
    mpLineEditSegmentLength->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mSegmentLength));
    mpLineEditSegmentLength->setMaximumWidth(40);

    mpCheckLoadWeightFromFile = new QCheckBox();
    mpLabelWeightFile = new QLabel("Weight File: ");
    mpLineEditWeightFile = new QLineEdit();
    mpBtnOpenWeightFile = new QPushButton("Load");
    if( mpParentWindow->mpViz->mMOPPInfo.mLoadWeightFile == false ) {
        mpCheckLoadWeightFromFile->setChecked(false);
        mpLineEditWeightFile->setReadOnly(true);
        mpBtnOpenWeightFile->setEnabled(false);
    }
    else {
        mpCheckLoadWeightFromFile->setChecked(true);
        mpLineEditWeightFile->setReadOnly(false);
        mpBtnOpenWeightFile->setEnabled(true);
    }
    mpCheckEnableInitWeightWSTransform = new QCheckBox();
    if(mpParentWindow->mpViz->mMOPPInfo.mInitWeightWSTransform==true) {
        mpCheckEnableInitWeightWSTransform->setChecked(true);
    }
    else {
        mpCheckEnableInitWeightWSTransform->setChecked(false);
    }
    mpLabelEnableInitWeightWSTransform = new QLabel("Init Weights WS Transform");
    connect(mpCheckLoadWeightFromFile, SIGNAL(clicked(bool)), this, SLOT(onLoadWeightToggled(bool)));
    connect(mpBtnOpenWeightFile, SIGNAL(clicked()), this, SLOT(onBtnOpenWeightFileClicked()));


    QHBoxLayout * minDistLayout = new QHBoxLayout();
    minDistLayout->addWidget(mpCheckMinDist);
    minDistLayout->addWidget(mpLabelMinDist);
    minDistLayout->addWidget(mpLabelSubProb);
    minDistLayout->addWidget(mpLineEditSubProb);
    minDistLayout->addWidget(mpLabelIterationNum);
    minDistLayout->addWidget(mpLineEditIterationNum);
    minDistLayout->addWidget(mpLabelSegmentLength);
    minDistLayout->addWidget(mpLineEditSegmentLength);

    QHBoxLayout * weightFileLayout = new QHBoxLayout();
    weightFileLayout->addWidget(mpCheckLoadWeightFromFile);
    weightFileLayout->addWidget(mpLabelWeightFile);
    weightFileLayout->addWidget(mpLineEditWeightFile);
    weightFileLayout->addWidget(mpBtnOpenWeightFile);

    mpLabelSparsityK = new QLabel(" Sparsity K: ");
    mpLineEditSparsityK = new QLineEdit();
    mpLineEditSparsityK->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mSparsityK));
    mpLineEditSparsityK->setMaximumWidth(40);

    mpLabelBIPenalty = new QLabel("BI Penalty:");;
    mpLineEditBIPenalty = new QLineEdit();
    mpLineEditBIPenalty->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mBoundaryIntersectionPenalty));
    mpLineEditBIPenalty->setMaximumWidth(40);

    mpLabelNewTreeCreationStep = new QLabel(" New Tree Creation Step: ");
    mpLineEditNewTreeCreationStep = new QLineEdit();
    mpLineEditNewTreeCreationStep->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mNewTreeCreationStep));
    mpLineEditNewTreeCreationStep->setMaximumWidth(40);

    mpLabelType = new QLabel(" Type: ");
    mpComboType = new QComboBox();
    mpComboType->addItem(WEIGHTED_SUM_STR);
    mpComboType->addItem(TCHEBYCHEFF_STR);
    mpComboType->addItem(BOUNDARY_INTERSECTION_STR);
    mpComboType->setCurrentIndex((int)mpParentWindow->mpViz->mMOPPInfo.mMethodType);

    QHBoxLayout * typeLayout = new QHBoxLayout();    
    typeLayout->addWidget(mpCheckEnableInitWeightWSTransform);
    typeLayout->addWidget(mpLabelEnableInitWeightWSTransform);
    typeLayout->addWidget(mpLabelSparsityK);
    typeLayout->addWidget(mpLineEditSparsityK);
    typeLayout->addWidget(mpLabelBIPenalty);
    typeLayout->addWidget(mpLineEditBIPenalty);
    typeLayout->addWidget(mpLabelNewTreeCreationStep);
    typeLayout->addWidget(mpLineEditNewTreeCreationStep);
    typeLayout->addWidget(mpLabelType);
    typeLayout->addWidget(mpComboType);

    mpListWidget = new QListWidget();
    mpListWidget->setViewMode(QListView::IconMode);
    for(std::vector<QString>::iterator it=mpParentWindow->mpViz->mMOPPInfo.mObjectiveFiles.begin();it!=mpParentWindow->mpViz->mMOPPInfo.mObjectiveFiles.end();it++) {
        QString filename = (*it);
        mpListWidget->addItem(filename);
    }
    mpListWidget->show();

    mpBtnAdd = new QPushButton(tr("Add"));
    mpBtnRemove = new QPushButton(tr("Remove"));
    mpBtnOK = new QPushButton(tr("OK"));
    mpBtnCancel = new QPushButton(tr("Cancel"));

    connect(mpBtnAdd, SIGNAL(clicked()), this, SLOT(onBtnAddClicked()));
    connect(mpBtnRemove, SIGNAL(clicked()), this, SLOT(onBtnRemoveClicked()));
    connect(mpBtnOK, SIGNAL(clicked()), this, SLOT(onBtnOKClicked()));
    connect(mpBtnCancel, SIGNAL(clicked()), this, SLOT(onBtnCancelClicked()));

    QHBoxLayout * buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(mpBtnAdd);
    buttonsLayout->addWidget(mpBtnRemove);
    buttonsLayout->addWidget(mpBtnOK);
    buttonsLayout->addWidget(mpBtnCancel);

    QVBoxLayout * mainLayout = new QVBoxLayout();
    mainLayout->addLayout(minDistLayout);
    mainLayout->addLayout(weightFileLayout);
    mainLayout->addLayout(typeLayout);
    mainLayout->addWidget(mpListWidget);
    mainLayout->addLayout(buttonsLayout);

    setWindowTitle("Config Objectives");

    setLayout(mainLayout);
}

void ConfigObjDialog::onBtnOKClicked() {
    updateConfiguration();
    close();
}

void ConfigObjDialog::onBtnCancelClicked() {
    close();
}

void ConfigObjDialog::onBtnAddClicked() {
   QString objFilename = QFileDialog::getOpenFileName(this,
                 tr("Open Objective File"), "./", tr("Objective Files (*.*)"));
   if (objFilename!="") {
       if(true==isCompatible(objFilename)) {
           mpListWidget->addItem(objFilename);
           repaint();
       }
       else {
           QMessageBox msg;
           msg.setText("Fitness distribution file is not compatible!");
           msg.exec();
       }
   }
   else {
       QMessageBox msg;
       msg.setText("Fitness distribution file is null!");
       msg.exec();
   }
}

void ConfigObjDialog::onBtnRemoveClicked() {
    qDeleteAll(mpListWidget->selectedItems());
}

void ConfigObjDialog::updateDisplay() {
    if(mpParentWindow) {
        if(mpParentWindow->mpViz) {
            if(mpParentWindow->mpViz->mMOPPInfo.mMinDistEnabled==true) {
                mpCheckMinDist->setChecked(true);
            }
            else {
                mpCheckMinDist->setChecked(false);
            }
            mpLineEditSubProb->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mSubproblemNum));
            mpLineEditSegmentLength->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mSegmentLength));
            mpLineEditIterationNum->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mMaxIterationNum));
            if(mpParentWindow->mpViz->mMOPPInfo.mObjectiveFiles.size()>0) {
                mpListWidget->clear();
                for(std::vector<QString>::iterator it= mpParentWindow->mpViz->mMOPPInfo.mObjectiveFiles.begin();
                    it!=mpParentWindow->mpViz->mMOPPInfo.mObjectiveFiles.end();it++) {
                    QString objFilename = (*it);
                    mpListWidget->addItem(objFilename);
                }
            }

            mpLineEditSparsityK->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mSparsityK));
            mpLineEditBIPenalty->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mBoundaryIntersectionPenalty));
            mpLineEditNewTreeCreationStep->setText(QString::number(mpParentWindow->mpViz->mMOPPInfo.mNewTreeCreationStep));
            mpComboType->setCurrentIndex((int)mpParentWindow->mpViz->mMOPPInfo.mMethodType);

            if(mpParentWindow->mpViz->mMOPPInfo.mLoadWeightFile==true) {
                mpCheckLoadWeightFromFile->setChecked(true);
                mpLineEditWeightFile->setReadOnly(false);
                mpBtnOpenWeightFile->setEnabled(true);
            }
            else {
                mpCheckLoadWeightFromFile->setChecked(false);
                mpLineEditWeightFile->setReadOnly(true);
                mpBtnOpenWeightFile->setEnabled(false);
            }
            mpLineEditWeightFile->setText(mpParentWindow->mpViz->mMOPPInfo.mWeightFile);

            if(mpParentWindow->mpViz->mMOPPInfo.mInitWeightWSTransform==true) {
                mpCheckEnableInitWeightWSTransform->setChecked(true);
            }
            else {
                mpCheckEnableInitWeightWSTransform->setChecked(false);
            }
        }
    }
}

void ConfigObjDialog::updateConfiguration() {
    int numObj = 0;
    if (mpCheckMinDist->isChecked()==true) {
        numObj += 1;
        mpParentWindow->mpViz->mMOPPInfo.mMinDistEnabled=true;
    }
    else {
        mpParentWindow->mpViz->mMOPPInfo.mMinDistEnabled=false;
    }

    mpParentWindow->mpViz->mMOPPInfo.mObjectiveFiles.clear();
    int count = mpListWidget->count();
    for(int i=0;i<count;i++) {
        QListWidgetItem * pItem = mpListWidget->item(i);
        mpParentWindow->mpViz->mMOPPInfo.mObjectiveFiles.push_back(pItem->text());
        numObj +=1;
    }

    mpParentWindow->mpViz->mMOPPInfo.mObjectiveNum = numObj;

    mpParentWindow->mpViz->mMOPPInfo.mMaxIterationNum = mpLineEditIterationNum->text().toInt();
    mpParentWindow->mpViz->mMOPPInfo.mSubproblemNum = mpLineEditSubProb->text().toInt();
    mpParentWindow->mpViz->mMOPPInfo.mSegmentLength = mpLineEditSegmentLength->text().toDouble();
    mpParentWindow->mpViz->mMOPPInfo.mSparsityK = mpLineEditSparsityK->text().toInt();
    mpParentWindow->mpViz->mMOPPInfo.mBoundaryIntersectionPenalty = mpLineEditBIPenalty->text().toDouble();
    mpParentWindow->mpViz->mMOPPInfo.mNewTreeCreationStep = mpLineEditNewTreeCreationStep->text().toInt();

    int type = mpComboType->currentIndex();
    mpParentWindow->mpViz->mMOPPInfo.mMethodType = (MORRF::MORRF_TYPE) type;

    if(mpCheckLoadWeightFromFile->isChecked()) {
        mpParentWindow->mpViz->mMOPPInfo.mLoadWeightFile = true;
        mpParentWindow->mpViz->mMOPPInfo.mWeightFile = mpLineEditWeightFile->text();
    }
    else {
        mpParentWindow->mpViz->mMOPPInfo.mLoadWeightFile = false;
        mpParentWindow->mpViz->mMOPPInfo.mWeightFile = "";
    }

    if(mpCheckEnableInitWeightWSTransform->isChecked()) {
        mpParentWindow->mpViz->mMOPPInfo.mInitWeightWSTransform = true;
    }
    else {
        mpParentWindow->mpViz->mMOPPInfo.mInitWeightWSTransform = false;
    }

}

bool ConfigObjDialog::isCompatible(QString fitnessFile) {

    if(true == fitnessFile.endsWith(".csv", Qt::CaseInsensitive)) {
        return true;
    }
    QPixmap pixmap(fitnessFile);
    if (pixmap.width()==mpParentWindow->mpViz->mMOPPInfo.mMapWidth
            && pixmap.height()==mpParentWindow->mpViz->mMOPPInfo.mMapHeight) {
        return true;
    }
    return false;
}

void ConfigObjDialog::onLoadWeightToggled(bool checked) {
    if( checked ) {
        mpLineEditWeightFile->setReadOnly(false);
        mpBtnOpenWeightFile->setEnabled(true);
    }
    else {
        mpLineEditWeightFile->setReadOnly(true);
        mpBtnOpenWeightFile->setEnabled(false);
    }
    update();
}

void ConfigObjDialog::onBtnOpenWeightFileClicked() {
    QString weightFilename = QFileDialog::getOpenFileName(this,
                  tr("Open Weight File"), "./", tr("Weight Files (*.*)"));
    mpLineEditWeightFile->setText(weightFilename);
    update();
}
