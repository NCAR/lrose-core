#include <QtWidgets>
#include <QPixmap>
#include <iostream>
// #include <QApplication>
#include <QLayout>

#include "FlowLayout.hh"
#include "Dialog.hh"
#include "ParameterColorDialog.hh"
#include "DialogOptionsWidget.hh"
#include "../HawkEye/ColorMap.hh"
#include "../HawkEye/ColorBar.hh"

#include "../ExamineEdit/SpreadSheetView.hh"
#include "../ExamineEdit/SpreadSheetController.hh"

#define MESSAGE \
    Dialog::tr("<p>Message boxes have a caption, a text, " \
               "and any number of buttons, each with standard or custom texts." \
               "<p>Click a button to close the message box. Pressing the Esc button " \
               "will activate the detected escape button (if any).")
#define MESSAGE_DETAILS \
    Dialog::tr("If a message box has detailed text, the user can reveal it " \
               "by pressing the Show Details... button.")
/*
class DialogOptionsWidget : public QGroupBox
{
public:
    explicit DialogOptionsWidget(QWidget *parent = 0);

    void addCheckBox(const QString &text, int value);
    void addSpacer();
    int value() const;

private:
    typedef QPair<QCheckBox *, int> CheckBoxEntry;
    QVBoxLayout *layout;
    QList<CheckBoxEntry> checkBoxEntries;
};

DialogOptionsWidget::DialogOptionsWidget(QWidget *parent) :
    QGroupBox(parent) , layout(new QVBoxLayout)
{
    setTitle(Dialog::tr("Options"));
    setLayout(layout);
}

void DialogOptionsWidget::addCheckBox(const QString &text, int value)
{
    QCheckBox *checkBox = new QCheckBox(text);
    layout->addWidget(checkBox);
    checkBoxEntries.append(CheckBoxEntry(checkBox, value));
}

void DialogOptionsWidget::addSpacer()
{
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
}

int DialogOptionsWidget::value() const
{
    int result = 0;
    foreach (const CheckBoxEntry &checkboxEntry, checkBoxEntries)
        if (checkboxEntry.first->isChecked())
            result |= checkboxEntry.second;
    return result;
}
*/

Dialog::Dialog(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *verticalLayout;
    if (QGuiApplication::styleHints()->showIsFullScreen() || QGuiApplication::styleHints()->showIsMaximized()) {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
        QGroupBox *groupBox = new QGroupBox(QGuiApplication::applicationDisplayName(), this);
        horizontalLayout->addWidget(groupBox);
        verticalLayout = new QVBoxLayout(groupBox);
    } else {
        verticalLayout = new QVBoxLayout(this);
    }

    // make a big label with the soloii just for prototyping
    QPixmap soloiiPixmap("/Users/brenda/Desktop/soloii_screenshot.png");
    QLabel *giantLabel = new QLabel();
    giantLabel->clear();
    giantLabel->setPixmap(soloiiPixmap);
    verticalLayout->addWidget(giantLabel);
    // TODO: also try this ...
    // palette->setBrush(QPalette::Background,(new QBrush((new QPixmap(":/1.jpg")))));
    // end big label of soloii image
    // setStyleSheet( "background-image:url(/Users/brenda/Desktop/soloii_screenshot.png);" );


    // add a right-click context menu to the image label
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(ShowContextMenu(const QPoint &)));

    // end right-click context menu

    QToolBox *toolbox = new QToolBox;
    verticalLayout->addWidget(toolbox);

    errorMessageDialog = new QErrorMessage(this);

    int frameStyle = QFrame::Sunken | QFrame::Panel;

    integerLabel = new QLabel;
    integerLabel->setFrameStyle(frameStyle);
    QPushButton *integerButton =
            new QPushButton(tr("QInputDialog::get&Int()"));

    doubleLabel = new QLabel;
    doubleLabel->setFrameStyle(frameStyle);
    QPushButton *doubleButton =
            new QPushButton(tr("QInputDialog::get&Double()"));

    itemLabel = new QLabel;
    itemLabel->setFrameStyle(frameStyle);
    QPushButton *itemButton = new QPushButton(tr("QInputDialog::getIte&m()"));

    textLabel = new QLabel;
    textLabel->setFrameStyle(frameStyle);
    QPushButton *textButton = new QPushButton(tr("QInputDialog::get&Text()"));

    multiLineTextLabel = new QLabel;
    multiLineTextLabel->setFrameStyle(frameStyle);
    QPushButton *multiLineTextButton = new QPushButton(tr("QInputDialog::get&MultiLineText()"));

    // Color Palette Editor ...
    QPushButton *saveButton = new QPushButton(tr("Save")); 

    centerColorLabel = new QLabel;
    centerColorLabel->setText(tr("Center"));
    centerColorLineEdit = new QLineEdit(); 
    // centerColorLineEdit->setValidator(new QDoubleValidator(-999.0, 999.0, 2, centerColorLineEdit);

    gridColorLabel = new QLabel;
    // colorLabel->setFrameStyle(frameStyle);
    gridColorLabel->setText(tr("Grid"));
    //QPushButton *colorButton = new QPushButton(tr("")); // QColorDialog::get&Color()"));
    gridColorButton = new QPushButton(tr("")); // QColorDialog::get&Color()"));
    gridColorButton->setFlat(true);
    //gridColorSample = new QLabel;

    boundaryColorLabel = new QLabel;
    boundaryColorLabel->setText(tr("Boundary"));
    boundaryColorButton = new QPushButton(tr("")); 
    boundaryColorButton->setFlat(true);

    exceededColorLabel = new QLabel;
    exceededColorLabel->setText(tr("Exceeded"));
    exceededColorButton = new QPushButton(tr(""));
    exceededColorButton->setFlat(true);

    missingColorLabel = new QLabel;
    missingColorLabel->setText(tr("Missing"));
    missingColorButton = new QPushButton(tr("")); 
    missingColorButton->setFlat(true);

    annotationColorLabel = new QLabel;
    annotationColorLabel->setText(tr("Annotation"));
    annotationColorButton = new QPushButton(tr("")); 
    annotationColorButton->setFlat(true);

    backgroundColorLabel = new QLabel;
    backgroundColorLabel->setText(tr("Background"));
    backgroundColorButton = new QPushButton(tr("")); 
    backgroundColorButton->setFlat(true);

    emphasisColorLabel = new QLabel;
    emphasisColorLabel->setText(tr("Emphasis"));
    emphasisColorButton = new QPushButton(tr(""));
    emphasisColorButton->setFlat(true);

    // end Color Palette Editor

    fontLabel = new QLabel;
    fontLabel->setFrameStyle(frameStyle);
    QPushButton *fontButton = new QPushButton(tr("QFontDialog::get&Font()"));

    directoryLabel = new QLabel;
    directoryLabel->setFrameStyle(frameStyle);
    QPushButton *directoryButton =
            new QPushButton(tr("QFileDialog::getE&xistingDirectory()"));

    openFileNameLabel = new QLabel;
    openFileNameLabel->setFrameStyle(frameStyle);
    QPushButton *openFileNameButton =
            new QPushButton(tr("QFileDialog::get&OpenFileName()"));

    openFileNamesLabel = new QLabel;
    openFileNamesLabel->setFrameStyle(frameStyle);
    QPushButton *openFileNamesButton =
            new QPushButton(tr("QFileDialog::&getOpenFileNames()"));

    saveFileNameLabel = new QLabel;
    saveFileNameLabel->setFrameStyle(frameStyle);
    QPushButton *saveFileNameButton =
            new QPushButton(tr("QFileDialog::get&SaveFileName()"));

    criticalLabel = new QLabel;
    criticalLabel->setFrameStyle(frameStyle);
    QPushButton *criticalButton =
            new QPushButton(tr("QMessageBox::critica&l()"));

    informationLabel = new QLabel;
    informationLabel->setFrameStyle(frameStyle);
    QPushButton *informationButton =
            new QPushButton(tr("QMessageBox::i&nformation()"));

    questionLabel = new QLabel;
    questionLabel->setFrameStyle(frameStyle);
    QPushButton *questionButton =
            new QPushButton(tr("QMessageBox::&question()"));

    warningLabel = new QLabel;
    warningLabel->setFrameStyle(frameStyle);
    QPushButton *warningButton = new QPushButton(tr("QMessageBox::&warning()"));

    errorLabel = new QLabel;
    errorLabel->setFrameStyle(frameStyle);
    QPushButton *errorButton =
            new QPushButton(tr("QErrorMessage::showM&essage()"));

    connect(integerButton, &QAbstractButton::clicked, this, &Dialog::setInteger);
    connect(doubleButton, &QAbstractButton::clicked, this, &Dialog::setDouble);
    connect(itemButton, &QAbstractButton::clicked, this, &Dialog::setItem);
    connect(textButton, &QAbstractButton::clicked, this, &Dialog::setText);
    connect(multiLineTextButton, &QAbstractButton::clicked, this, &Dialog::setMultiLineText);
    //connect(centerColorLineEdit, &QLineEdit::textEdited, this, &Dialog::setCenterPoint);

    // connections for Parameters + Color Editor
    connect(saveButton, &QAbstractButton::clicked, this, &Dialog::saveColorScale);

    connect(gridColorButton, &QAbstractButton::clicked, this, &Dialog::setGridColor);
    connect(boundaryColorButton, &QAbstractButton::clicked, this, &Dialog::setBoundaryColor);
    connect(exceededColorButton, &QAbstractButton::clicked, this, &Dialog::setExceededColor);
    connect(missingColorButton, &QAbstractButton::clicked, this, &Dialog::setMissingColor);
    connect(annotationColorButton, &QAbstractButton::clicked, this, &Dialog::setAnnotationColor);
    connect(backgroundColorButton, &QAbstractButton::clicked, this, &Dialog::setBackgroundColor);
    connect(emphasisColorButton, &QAbstractButton::clicked, this, &Dialog::setEmphasisColor);
    //connect(colorSample, &QAbstractLabel::clicked, this, &Dialog::setColor);


    connect(fontButton, &QAbstractButton::clicked, this, &Dialog::setFont);
    connect(directoryButton, &QAbstractButton::clicked,
            this, &Dialog::setExistingDirectory);
    connect(openFileNameButton, &QAbstractButton::clicked,
            this, &Dialog::setOpenFileName);
    connect(openFileNamesButton, &QAbstractButton::clicked,
            this, &Dialog::setOpenFileNames);
    connect(saveFileNameButton, &QAbstractButton::clicked,
            this, &Dialog::setSaveFileName);
    connect(criticalButton, &QAbstractButton::clicked, this, &Dialog::criticalMessage);
    connect(informationButton, &QAbstractButton::clicked,
            this, &Dialog::informationMessage);
    connect(questionButton, &QAbstractButton::clicked, this, &Dialog::questionMessage);
    connect(warningButton, &QAbstractButton::clicked, this, &Dialog::warningMessage);
    connect(errorButton, &QAbstractButton::clicked, this, &Dialog::errorMessage);

    QWidget *page = new QWidget;
    QGridLayout *layout = new QGridLayout(page);
    layout->setColumnStretch(1, 1);
    layout->setColumnMinimumWidth(1, 250);
    layout->addWidget(integerButton, 0, 0);
    layout->addWidget(integerLabel, 0, 1);
    layout->addWidget(doubleButton, 1, 0);
    layout->addWidget(doubleLabel, 1, 1);
    layout->addWidget(itemButton, 2, 0);
    layout->addWidget(itemLabel, 2, 1);
    layout->addWidget(textButton, 3, 0);
    layout->addWidget(textLabel, 3, 1);
    layout->addWidget(multiLineTextButton, 4, 0);
    layout->addWidget(multiLineTextLabel, 4, 1);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding), 5, 0);
    toolbox->addItem(page, tr("Input Dialogs"));

    const QString doNotUseNativeDialog = tr("Do not use native dialog");

    page = new QWidget;
    layout = new QGridLayout(page);
    layout->setColumnStretch(1, 1);
    
    layout->addWidget(centerColorLabel, 0, 0);
    layout->addWidget(centerColorLineEdit, 0, 1);

    int base = 2; 
    layout->addWidget(gridColorButton, base+0, 1);
    layout->addWidget(gridColorLabel, base+0, 0);
    layout->addWidget(boundaryColorButton, base+1, 1);
    layout->addWidget(boundaryColorLabel, base+1, 0);
    layout->addWidget(exceededColorButton, base+2, 1);
    layout->addWidget(exceededColorLabel, base+2, 0);
    layout->addWidget(missingColorButton, base+3, 1);
    layout->addWidget(missingColorLabel, base+3, 0);
    layout->addWidget(annotationColorButton, base+4, 1);
    layout->addWidget(annotationColorLabel, base+4, 0);
    layout->addWidget(backgroundColorButton, base+5, 1);
    layout->addWidget(backgroundColorLabel, base+5, 0);
    layout->addWidget(emphasisColorButton, base+6, 1);
    layout->addWidget(emphasisColorLabel, base+6, 0);
    layout->addWidget(saveButton, base+7, 0);
    toolbox->addItem(page, tr("Color Dialog"));
   /* 
    // make the color map chooser

    page = new QWidget;
    // QHBoxLayout *flowLayout = new QHBoxLayout(page);
    FlowLayout *flowLayout = new FlowLayout(page);
   
    // make the color map picker
    
    ColorMap *cmap = new ColorMap(0.0, 100.0, "default");
    ColorBar *colorBar = new ColorBar(1, cmap, this);
    QPixmap *pixmap = colorBar->getPixmap();
    QLabel *cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(*pixmap);
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "rainbow");
    colorBar = new ColorBar(1, cmap, this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(*pixmap);
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "eldoraDbz");
    colorBar = new ColorBar(1, cmap, this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(*pixmap);
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "spolDbz");
    colorBar = new ColorBar(1, cmap, this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(*pixmap);
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "eldoraVel");
    colorBar = new ColorBar(1, cmap, this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(*pixmap);
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "spolVel");
    colorBar = new ColorBar(1, cmap, this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(*pixmap);
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "spolDiv");
    colorBar = new ColorBar(1, cmap, this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(*pixmap);
    flowLayout->addWidget(cmapLabel);
*/

    // colorDialogOptionsWidget = new DialogOptionsWidget;
    // colorDialogOptionsWidget->addCheckBox(doNotUseNativeDialog, QColorDialog::DontUseNativeDialog);
    // colorDialogOptionsWidget->addCheckBox(tr("Show alpha channel") , QColorDialog::ShowAlphaChannel);
    // colorDialogOptionsWidget->addCheckBox(tr("No buttons") , QColorDialog::NoButtons);
    // layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding), 1, 0);
    // layout->addWidget(colorDialogOptionsWidget, 2, 0, 1 ,2);

    toolbox->addItem(page, tr("Color Map Chooser"));

    // end - color map chooser

    page = new QWidget;
    layout = new QGridLayout(page);
    layout->setColumnStretch(1, 1);
    layout->addWidget(fontButton, 0, 0);
    layout->addWidget(fontLabel, 0, 1);
    fontDialogOptionsWidget = new DialogOptionsWidget;
    fontDialogOptionsWidget->addCheckBox(doNotUseNativeDialog, QFontDialog::DontUseNativeDialog);
    fontDialogOptionsWidget->addCheckBox(tr("Show scalable fonts"), QFontDialog::ScalableFonts);
    fontDialogOptionsWidget->addCheckBox(tr("Show non scalable fonts"), QFontDialog::NonScalableFonts);
    fontDialogOptionsWidget->addCheckBox(tr("Show monospaced fonts"), QFontDialog::MonospacedFonts);
    fontDialogOptionsWidget->addCheckBox(tr("Show proportional fonts"), QFontDialog::ProportionalFonts);
    fontDialogOptionsWidget->addCheckBox(tr("No buttons") , QFontDialog::NoButtons);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding), 1, 0);
    layout->addWidget(fontDialogOptionsWidget, 2, 0, 1 ,2);
    toolbox->addItem(page, tr("Font Dialog"));

    page = new QWidget;
    layout = new QGridLayout(page);
    layout->setColumnStretch(1, 1);
    layout->addWidget(directoryButton, 0, 0);
    layout->addWidget(directoryLabel, 0, 1);
    layout->addWidget(openFileNameButton, 1, 0);
    layout->addWidget(openFileNameLabel, 1, 1);
    layout->addWidget(openFileNamesButton, 2, 0);
    layout->addWidget(openFileNamesLabel, 2, 1);
    layout->addWidget(saveFileNameButton, 3, 0);
    layout->addWidget(saveFileNameLabel, 3, 1);
    fileDialogOptionsWidget = new DialogOptionsWidget;
    fileDialogOptionsWidget->addCheckBox(doNotUseNativeDialog, QFileDialog::DontUseNativeDialog);
    fileDialogOptionsWidget->addCheckBox(tr("Show directories only"), QFileDialog::ShowDirsOnly);
    fileDialogOptionsWidget->addCheckBox(tr("Do not resolve symlinks"), QFileDialog::DontResolveSymlinks);
    fileDialogOptionsWidget->addCheckBox(tr("Do not confirm overwrite"), QFileDialog::DontConfirmOverwrite);
    fileDialogOptionsWidget->addCheckBox(tr("Do not use sheet"), QFileDialog::DontUseSheet);
    fileDialogOptionsWidget->addCheckBox(tr("Readonly"), QFileDialog::ReadOnly);
    fileDialogOptionsWidget->addCheckBox(tr("Hide name filter details"), QFileDialog::HideNameFilterDetails);
    fileDialogOptionsWidget->addCheckBox(tr("Do not use custom directory icons (Windows)"), QFileDialog::DontUseCustomDirectoryIcons);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding), 4, 0);
    layout->addWidget(fileDialogOptionsWidget, 5, 0, 1 ,2);
    toolbox->addItem(page, tr("File Dialogs"));

    page = new QWidget;
    layout = new QGridLayout(page);
    layout->setColumnStretch(1, 1);
    layout->addWidget(criticalButton, 0, 0);
    layout->addWidget(criticalLabel, 0, 1);
    layout->addWidget(informationButton, 1, 0);
    layout->addWidget(informationLabel, 1, 1);
    layout->addWidget(questionButton, 2, 0);
    layout->addWidget(questionLabel, 2, 1);
    layout->addWidget(warningButton, 3, 0);
    layout->addWidget(warningLabel, 3, 1);
    layout->addWidget(errorButton, 4, 0);
    layout->addWidget(errorLabel, 4, 1);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding), 5, 0);
    toolbox->addItem(page, tr("Message Boxes"));

    setWindowTitle(QGuiApplication::applicationDisplayName());
}


void Dialog::ShowContextMenu(const QPoint &pos) 
{
   QMenu contextMenu(tr("Context menu"), this);

   QAction action1("Cancel", this);
   connect(&action1, SIGNAL(triggered()), this, SLOT(contextMenuCancel()));
   contextMenu.addAction(&action1);

   QAction action2("Sweepfiles", this);
   connect(&action2, SIGNAL(triggered()), this, SLOT(contextMenuSweepfiles()));
   contextMenu.addAction(&action2);

   QAction action3("Parameters + Colors", this);
   connect(&action3, SIGNAL(triggered()), this, SLOT(contextMenuParameterColors()));
   contextMenu.addAction(&action3);

   QAction action4("View", this);
   connect(&action4, SIGNAL(triggered()), this, SLOT(contextMenuView()));
   contextMenu.addAction(&action4);

   QAction action5("Editor", this);
   connect(&action5, SIGNAL(triggered()), this, SLOT(contextMenuEditor()));
   contextMenu.addAction(&action5);

   QAction action6("Examine", this);
   connect(&action6, SIGNAL(triggered()), this, SLOT(contextMenuExamine()));
   contextMenu.addAction(&action6);

   QAction action7("Data Widget", this);
   connect(&action7, SIGNAL(triggered()), this, SLOT(contextMenuDataWidget()));
   contextMenu.addAction(&action7);


   contextMenu.exec(mapToGlobal(pos));
}

void Dialog::setInteger()
{
    bool ok;
    int i = QInputDialog::getInt(this, tr("QInputDialog::getInteger()"),
                                 tr("Percentage:"), 25, 0, 100, 1, &ok);
    if (ok)
        integerLabel->setText(tr("%1%").arg(i));
}

void Dialog::setDouble()
{
    bool ok;
    double d = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Amount:"), 37.56, -10000, 10000, 2, &ok);
    if (ok)
        doubleLabel->setText(QString("$%1").arg(d));
}

void Dialog::setItem()
{
    QStringList items;
    items << tr("Spring") << tr("Summer") << tr("Fall") << tr("Winter");

    bool ok;
    QString item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
                                         tr("Season:"), items, 0, false, &ok);
    if (ok && !item.isEmpty())
        itemLabel->setText(item);
}

void Dialog::setText()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("User name:"), QLineEdit::Normal,
                                         QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty())
        textLabel->setText(text);
}

void Dialog::setMultiLineText()
{
    bool ok;
    QString text = QInputDialog::getMultiLineText(this, tr("QInputDialog::getMultiLineText()"),
                                                  tr("Address:"), "John Doe\nFreedom Street", &ok);
    if (ok && !text.isEmpty())
        multiLineTextLabel->setText(text);
}

void Dialog::setColor()
{
/*
    const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this, "Select Color", options);

    if (color.isValid()) {
        // colorLabel->setText(color.name());
        // colorSample->setPalette(QPalette(color));
        // colorSample->setAutoFillBackground(true);
        colorButton->setPalette(QPalette(color));
        colorButton->setAutoFillBackground(true);
    }
*/
    ParameterColorDialog parameterColorDialog(this); //  = new ParameterColorDialog(this);
    parameterColorDialog.exec();
    bool changed = parameterColorDialog.getChanges();    
}

void Dialog::ExamineEdit() {
    SpreadSheetView *sheetView;

    sheetView = new SpreadSheetView();
    SpreadSheetController sheetControl(sheetView);
    sheetView->show();
    sheetView->layout()->setSizeConstraint(QLayout::SetFixedSize);

}

void Dialog::setCenterPoint()
{
  QString qvalue;
  bool ok = false;
  qvalue = centerColorLineEdit->text();
  double value = qvalue.toDouble(&ok);

  if (ok) {
    printf("value entered %g\n", value);
  } else {
    printf("unrecognized value entered\n");
  }
  //qvalue = centerColorLineEdit->text();
  //std::string value = qvalue.toStdString();
  //sscanf(value.c_str(),"%g", &newCenterPoint);
}

void Dialog::saveColorScale() {

  QString qvalue;
  bool ok = false;
  qvalue = centerColorLineEdit->text();
  double value = qvalue.toDouble(&ok);

  if (ok) {
    printf("value entered %g\n", value);
  } else {
    printf("unrecognized value entered\n");
  }

}

void Dialog::setGridColor()
{
    const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this, "Select Color", options);

    if (color.isValid()) {
        gridColorButton->setPalette(QPalette(color));
        gridColorButton->setAutoFillBackground(true);
    }
}

void Dialog::setBoundaryColor()
{
    const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this, "Select Color", options);

    if (color.isValid()) {
        boundaryColorButton->setPalette(QPalette(color));
        boundaryColorButton->setAutoFillBackground(true);
    }
}

void Dialog::setExceededColor()
{
    //const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        boundaryColorButton->setPalette(QPalette(color));
        boundaryColorButton->setAutoFillBackground(true);
    }
}

void Dialog::setMissingColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        missingColorButton->setPalette(QPalette(color));
        missingColorButton->setAutoFillBackground(true);
    }
}

void Dialog::setAnnotationColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        annotationColorButton->setPalette(QPalette(color));
        annotationColorButton->setAutoFillBackground(true);
    }
}

void Dialog::setBackgroundColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        backgroundColorButton->setPalette(QPalette(color));
        backgroundColorButton->setAutoFillBackground(true);
    }
}

void Dialog::setEmphasisColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        emphasisColorButton->setPalette(QPalette(color));
        emphasisColorButton->setAutoFillBackground(true);
    }
}

void Dialog::contextMenuCancel() 
{
    notImplemented();
}

void Dialog::contextMenuSweepfiles()
{
    notImplemented();
}

void Dialog::contextMenuParameterColors()
{
    setColor();
}

void Dialog::contextMenuView()
{
    notImplemented();
}

void Dialog::contextMenuEditor()
{
    notImplemented();
}

void Dialog::contextMenuExamine()
{
    // notImplemented();
    ExamineEdit();
}

void Dialog::contextMenuDataWidget()
{
    notImplemented();
}

void Dialog::setFont()
{
    const QFontDialog::FontDialogOptions options = QFlag(fontDialogOptionsWidget->value());
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QFont(fontLabel->text()), this, "Select Font", options);
    if (ok) {
        fontLabel->setText(font.key());
        fontLabel->setFont(font);
    }
}

void Dialog::setExistingDirectory()
{
    QFileDialog::Options options = QFlag(fileDialogOptionsWidget->value());
    options |= QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString directory = QFileDialog::getExistingDirectory(this,
                                tr("QFileDialog::getExistingDirectory()"),
                                directoryLabel->text(),
                                options);
    if (!directory.isEmpty())
        directoryLabel->setText(directory);
}

void Dialog::setOpenFileName()
{
    const QFileDialog::Options options = QFlag(fileDialogOptionsWidget->value());
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("QFileDialog::getOpenFileName()"),
                                openFileNameLabel->text(),
                                tr("All Files (*);;Text Files (*.txt)"),
                                &selectedFilter,
                                options);
    if (!fileName.isEmpty())
        openFileNameLabel->setText(fileName);
}

void Dialog::setOpenFileNames()
{
    const QFileDialog::Options options = QFlag(fileDialogOptionsWidget->value());
    QString selectedFilter;
    QStringList files = QFileDialog::getOpenFileNames(
                                this, tr("QFileDialog::getOpenFileNames()"),
                                openFilesPath,
                                tr("All Files (*);;Text Files (*.txt)"),
                                &selectedFilter,
                                options);
    if (files.count()) {
        openFilesPath = files[0];
        openFileNamesLabel->setText(QString("[%1]").arg(files.join(", ")));
    }
}

void Dialog::setSaveFileName()
{
    const QFileDialog::Options options = QFlag(fileDialogOptionsWidget->value());
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                tr("QFileDialog::getSaveFileName()"),
                                saveFileNameLabel->text(),
                                tr("All Files (*);;Text Files (*.txt)"),
                                &selectedFilter,
                                options);
    if (!fileName.isEmpty())
        saveFileNameLabel->setText(fileName);
}

void Dialog::notImplemented()
{
    errorMessageDialog->showMessage(
            tr("This option is not implemented yet."));

    errorLabel->setText(tr("If the box is unchecked, the message "
                           "won't appear again."));
}


void Dialog::criticalMessage()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::critical(this, tr("QMessageBox::critical()"),
                                    MESSAGE,
                                    QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
    if (reply == QMessageBox::Abort)
        criticalLabel->setText(tr("Abort"));
    else if (reply == QMessageBox::Retry)
        criticalLabel->setText(tr("Retry"));
    else
        criticalLabel->setText(tr("Ignore"));
}

void Dialog::informationMessage()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(this, tr("QMessageBox::information()"), MESSAGE);
    if (reply == QMessageBox::Ok)
        informationLabel->setText(tr("OK"));
    else
        informationLabel->setText(tr("Escape"));
}

void Dialog::questionMessage()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("QMessageBox::question()"),
                                    MESSAGE,
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
        questionLabel->setText(tr("Yes"));
    else if (reply == QMessageBox::No)
        questionLabel->setText(tr("No"));
    else
        questionLabel->setText(tr("Cancel"));
}

void Dialog::warningMessage()
{
    QMessageBox msgBox(QMessageBox::Warning, tr("QMessageBox::warning()"),
                       MESSAGE, 0, this);
    msgBox.setDetailedText(MESSAGE_DETAILS);
    msgBox.addButton(tr("Save &Again"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("&Continue"), QMessageBox::RejectRole);
    if (msgBox.exec() == QMessageBox::AcceptRole)
        warningLabel->setText(tr("Save Again"));
    else
        warningLabel->setText(tr("Continue"));

}

void Dialog::errorMessage()
{
    errorMessageDialog->showMessage(
            tr("This dialog shows and remembers error messages. "
               "If the checkbox is checked (as it is by default), "
               "the shown message will be shown again, "
               "but if the user unchecks the box the message "
               "will not appear again if QErrorMessage::showMessage() "
               "is called with the same message."));
    errorLabel->setText(tr("If the box is unchecked, the message "
                           "won't appear again."));
}
