/*! \class GlobalReplaceDialog
 *
 *  \brief This class is used to handle the gui functionality of global replace dialog box
 *         Whenever user saves the page and makes changes to words, this dialog box will appear and
 *         show the words which can be globally replaced by the user.
 */

#include "globalreplacedialog.h"
#include "ui_globalreplacedialog.h"
#include <QDialogButtonBox>
#include <QDebug>
#include "globalreplacepreview.h"
#include "ui_globalreplacepreview.h"
#include "globalreplaceinformation.h"
#include <QMessageBox>
#include "crashlog.h"
#include <mainwindow.h>

/*!
 * \fn GlobalReplaceDialog::GlobalReplaceDialog
 * \brief In this constructor function we initialize the UI and setup the layout, margins, alignment,etc.
 * \param replacedWords,parent
 */
GlobalReplaceDialog::GlobalReplaceDialog(QVector <QString> replacedWords, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalReplaceDialog)
{
    ui->setupUi(this);
    qInstallMessageHandler(crashlog::myMessageHandler);
    setWindowTitle("Select the words you want to replace globally");
    displayOriginalList(replacedWords);
    //ui->listWidget->insertItem(,"Replacement Words");
    ui->pushButton_2->setVisible(false);

    QObject::connect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(highlightChecked(QListWidgetItem*)));

    //! Whenever user clicks on global replace preview, the words that are checked are fetched and those
    //! words are passed to show global replace preview.
    QObject::connect(this , SIGNAL(fetchCheckedlist(QMap<QString,QString>,  QVector<int>)), parent, SLOT(globalReplacePreviewfn(QMap<QString,QString>,QVector<int>)));

    //! Set layout, margins, alignment
    QVBoxLayout *listLayout = new QVBoxLayout;
    ui->listWidget->setLayout(listLayout);
    ui->groupBox->setVisible(false);
    ui->ReplaceInAllPagesListWidget->setLayout(listLayout);
}

/*!
 * \fn GlobalReplaceDialog::~GlobalReplaceDialog
 * \brief Destructor
 */
GlobalReplaceDialog::~GlobalReplaceDialog()
{
    rightPaneCheckboxes.clear();
    delete ui;
}

/*!
 * \fn GlobalReplaceDialog::getFilteredGlobalReplacementMap
 * \brief Returns the processed globalreplacement map which contain the key value pairs of word before
 * replacement and after it is replaced.
 * \return Filtered global replacement map
 */
QMap <QString, QString> GlobalReplaceDialog::getFilteredGlobalReplacementMap(){
    return this->filteredGlobalReplacementMap;
}

/*!
 * \fn GlobalReplaceDialog::displayOriginalList
 * \brief This function is used to show the checkboxes for words that can be globally replaced in
 *        list format.
 * \param ReplacedWords
 */
void GlobalReplaceDialog::displayOriginalList(QVector <QString> replacedWords)
{
    //! We get words in the format old Word => new Word. (See editdistance.cpp for more info)
    //! We run the loop of the list of such strings and we separate them using QRegExp and add
    //! to the widget.
    //replacedWords1 stores only those replaced words which are not composed of only special symbols
    QVector <QString> replacedWords1(0);
    for (int i = 0; i < replacedWords.size(); ++i){
        QRegExp sep("\\s*=>*");
        QString item_ = replacedWords[i];
        QString word_ = replacedWords[i].split(sep).first();
        QString special_symbols = "~`!@#$%^&*()-+={}[]|\ \"/:;'<>,.?;";
        int replaceFlag = 0;
        for(int k=0;k<word_.size();k++){
            int count =0;
            for(int l=0;l<special_symbols.size();l++){
                if(word_[k] != special_symbols[l]){
                    count +=1;
                }
                if(count == special_symbols.size()){
                    replaceFlag = 1;
                    break;
                }
            }
            if(replaceFlag == 1)
                break;
        }
        if(replaceFlag == 1){
            replacedWords1.append(item_);
        }
    }
    //if we use replacedWords here, it will show all the replaced words
    //globalreplacedialog irrespective of whether word is made up of special symbols only or not
    //replacedWords1 is a filtered vector which excludes the words which can cause set corruption
    for (int i = 0; i < replacedWords1.size(); ++i){
        QRegExp sep("\\s*=>*");
        QStringList changedList = replacedWords1[i].split(sep);
        //QStringList changedList = replacedWords[i].split(" ");
        ui->listWidget ->addItem(changedList[0]+ " -> " + changedList[1]);
        QString test_word = changedList[0];

        leftPaneCheckboxesStates.push_back(0);

        // NEW IMPLEMENTATION OF RIGHT PANE USING QListWidget
        ui->ReplaceInAllPagesListWidget->addItem("");
        QListWidgetItem* item = ui->ReplaceInAllPagesListWidget->item(i);
        rightPaneCheckboxes.push_back(item);
    }

    //! spawn  checkboxes for list
    QListWidgetItem *item = 0, *rightPaneListItem = 0;
    Qt::ItemFlags flags;
    for(int i = 0; i < ui ->listWidget->count(); ++i){
        item = ui->listWidget->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);

        rightPaneListItem = ui->ReplaceInAllPagesListWidget->item(i);
        rightPaneListItem->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        rightPaneListItem->setCheckState(Qt::Unchecked);

        flags = rightPaneListItem->flags();
        flags = flags & (~(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable));
        rightPaneListItem->setFlags(flags);
    }
    //! Whenever user checks the word for global replace, the adjacent checkbox
    //! (to ask user whether to replace in all pages or only unedited pages) becomes visible.
    //! The connect function does the necessary syncing.

    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(leftCheckBoxStateChanged(QListWidgetItem*)));
    connect(ui->ReplaceInAllPagesListWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(rightCheckBoxStateChanged()));
}


/*!
 * \fn GlobalReplaceDialog::highlightChecked
 * \brief We change the color of list to lightblue when it is checked and when it is unchecked.
 * This is done to make the GUI elegant.
 * \param ListItem
 */
void GlobalReplaceDialog::highlightChecked(QListWidgetItem *item){
    if(item->checkState() == Qt::Checked)
        item->setBackgroundColor(QColor("#add8e6"));
    else
        item->setBackgroundColor(QColor("#ffffff"));
}

/*!
 * \fn GlobalReplaceDialog::on_applyButton_clicked
 * \brief Defines what happens when user clicks on apply to global replace.
 *        It puts the words in the global replace map and applies global replace.
 */
void GlobalReplaceDialog::on_applyButton_clicked()
{
    QMessageBox replace;
    replace.setWindowTitle("Save and Replace");
    replace.setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowCloseButtonHint);
    replace.setIcon(QMessageBox::Question);
    replace.setInformativeText("Selected words will be saved and replaced");
    QPushButton *confirmButton = replace.addButton(tr("Confirm"),QMessageBox::AcceptRole);
    QPushButton *cancelButton = replace.addButton(tr("Cancel"),QMessageBox::ActionRole);

    replace.exec();
    if(replace.clickedButton() == confirmButton)
    {
      applyButtonIsClicked = true;
      QList<QListWidgetItem *> items = ui->listWidget->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard); //get all items

      foreach (QListWidgetItem *item, items){
          if(item->checkState() == Qt::Checked){
              QRegExp sep("\\s* -> *");
              QStringList string = item->text().split(sep);
              //QStringList string = item->text().split(" ");
              this->filteredGlobalReplacementMap[string[0]] = string[1];
          }
      }
      this->close();
    }
    if(replace.clickedButton() == cancelButton){
        replace.close();
    }

}

/*!
 * \fn GlobalReplaceDialog::uncheckedItemsList
 * \brief This function returns the map of unchecked items list.
 * \param item
 * \return UncheckedItemMap
 */
QMap <QString, QString> GlobalReplaceDialog::uncheckedItemsList()
{
    //! We first get the list of all checkbox item labels.
    QList<QListWidgetItem *> items = ui->listWidget->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard);

    //! Then we loop through this list and get unchecked items
    //! Then we separate key from the value i.e (oldword and newWord) and we add that to the map of
    //! unchecked items.
    foreach (QListWidgetItem *item, items){
    if(item->checkState() == Qt::Unchecked){
        QRegExp sep("\\s* -> *");
        QStringList string = item->text().split(sep);
        this->uncheckedItemsListMap[string[0]] = string[1];
       }
   }
    //! We return this map to the calling function.
    return this->uncheckedItemsListMap;
}


/*!
 * \fn GlobalReplaceDialog::leftCheckBoxStateChanged
 * \brief This function is a SLOT which receives signal from listWidget when an item is selected
 * \details
 * This function is called by QT whenever user checks or unchecks the left hand side checkbox
 * in the global replace window.
 * If user clicks on lecft checkbox, then right hand side checkbox gets visible.
 * \param item Address of QListWidgetItem is passed
 */
void GlobalReplaceDialog::leftCheckBoxStateChanged(QListWidgetItem* item)
{
    int itemRow;
    itemRow = ui->listWidget->row(item);
    Qt::ItemFlags flags = ui->ReplaceInAllPagesListWidget->item(itemRow)->flags();
    //To uncheck 'Select All' button when some sub-item is unchecked
    QListWidgetItem* item_ = 0;
    int count_=0;
    for(int j = 0; j < ui ->listWidget->count(); ++j){
        item_ = ui->listWidget->item(j);
        if(item_->checkState() == Qt::Checked){
            count_ ++;
        }
    }
    if(count_ == ui ->listWidget->count()){
        ui->checkBox->setCheckState(Qt::Checked);
    }
    else if( count_ != ui ->listWidget->count() ){
        ui->checkBox->setCheckState(Qt::Unchecked);
    }
    count_ = 0;
    //! This enables the right hand side pane, which was invisible when no words were checked.
    //! Enables the checkbox for the respective left hand side word checkbox checked.
    if (item->checkState() == Qt::Checked)
    {
        leftPaneCheckboxesStates[itemRow] = 1;
        ui->groupBox->setVisible(true);
        flags = flags | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
        ui->ReplaceInAllPagesListWidget->item(itemRow)->setFlags(flags);
    }
    //! If the word on the left side is unchecked then the right checkbox gets disabled.
    else if (item->checkState() == Qt::Unchecked)
    {
        leftPaneCheckboxesStates[itemRow] = 0;
        flags = flags & (~(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable));
        ui->ReplaceInAllPagesListWidget->item(itemRow)->setFlags(flags);
        ui->ReplaceInAllPagesListWidget->item(itemRow)->setCheckState(Qt::Unchecked);
        //! However if some other word is checked then we still keep the right hand side groupbox pane
        //! visible, only those words are not selected their right hand side checkbox is disabled.
        //! Thus we check if some other checkbox is checked or not?? If yes the groupbox is visible else
        //! invisible
        for (int i = 0; i < leftPaneCheckboxesStates.size(); i++)
        {
            if ( leftPaneCheckboxesStates.at(i) == 1 )
            {
                ui->groupBox->setVisible(true);
                return;
            }
        }
        ui->groupBox->setVisible(false);
    }

}

/*!
 * \fn GlobalReplaceDialog::getStatesOfCheckboxes
 * \brief This gets the state of checkboxes at the right hand side of the pane and returns the vector.
 * \return States of checkboxes
 */
QVector<int> GlobalReplaceDialog::getStatesOfCheckboxes()
{
    QVector<int> statesOfRightCheckboxes;

    //! Loop through all checkboxes, if left hand side checkboxed is checked
    //! and right hand side of checkbox is also checked then push 1 else push 0
    for (int i = 0; i < leftPaneCheckboxesStates.size(); i++)
    {
        if (leftPaneCheckboxesStates.at(i) == 1) {
            if (rightPaneCheckboxes.at(i)->checkState() == Qt::Checked)
                statesOfRightCheckboxes.push_back(1);
            else
                statesOfRightCheckboxes.push_back(0);
        }
    }
    return statesOfRightCheckboxes;
}


/*!
 * \fn GlobalReplaceDialog::clicked_applyButton
 * \brief Returns if apply button is clicked
 * \brief If apply button was clicked or not
 */
bool GlobalReplaceDialog::clicked_applyButton()
{
    return applyButtonIsClicked;
}

/*!
 * \fn GlobalReplaceDialog::on_previewButton_clicked
 * \brief Defines what happens when preview button is clicked. Gets the checkboxes are clicked and
 * emits a signal which causes preview dialog to show up.
 */
void GlobalReplaceDialog::on_previewButton_clicked()
{
    QMap <QString, QString> obj;
    QList<QListWidgetItem *> items = ui->listWidget->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard); //get all items
    QVector<int> allPages;

    //! Loops through each item and splits the string such that when oldword newword is the format of
    //! the string then we get key as oldword and value as new word.
    foreach (QListWidgetItem *item, items)
    {
        if(item->checkState() == Qt::Checked)
          {
            QStringList string = item->text().split("->");
            obj[string[0]] = string[1];
          }
    }
    //! This will check if the words are selected to be replaced in unedited pages or all pages as well
    //! This data is stored in vector.
    if(obj.size()>0)
    {
      allPages= getStatesOfCheckboxes();
    }
    //! Once the signal is emitted, it _implicitly_ calls the global replace preview function and makes the
    //! dialog box show up. All pages vector defines whether the preview function show preview for all pages
    //! for that particular word or just unedited pages.
    emit fetchCheckedlist(obj,allPages);
}

/*!
 * \fn GlobalReplaceDialog::on_pushButton_clicked
 * \brief This opens the global replace information dialog box.
 * Whenever users click on the 𝒊 symbol they can see what each button does in global replace,
 * thus better equipped to use the feature.
 */
void GlobalReplaceDialog::on_pushButton_clicked()
{
    globalReplaceInformation info(this);
    info.exec();
}

/*!
 * \fn GlobalReplaceDialog::on_checkBox_clicked
 * \brief This function is triggered by "Select All" button(on left side) which checks all the checkboxes on the left.
 */
void GlobalReplaceDialog::on_checkBox_clicked()
{
    QListWidgetItem* item = 0;
    QList<QListWidgetItem*> list;
    for(int i = 0; i < ui ->listWidget->count(); ++i){
        item = ui->listWidget->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if(ui->checkBox->checkState() == Qt::Unchecked){
            item->setCheckState(Qt::Unchecked);
            leftCheckBoxStateChanged(item);
        }
        else if(ui->checkBox->checkState() == Qt::Checked){
            item->setCheckState(Qt::Checked);
            list.append(item);
        }
    }
    for(int j=0;j<list.size();++j){
        leftCheckBoxStateChanged(list[j]);
    }
}

/*!
 * \fn GlobalReplaceDialog::on_checkBox_2_clicked
 * \brief This function is triggered by the "Select All" button(on right side) which checks all enabled checkboxes on the right .
 */
void GlobalReplaceDialog::on_checkBox_2_clicked()
{
    QListWidgetItem* item = 0;
    Qt::ItemFlags flags;

    for(int i = 0; i < ui ->ReplaceInAllPagesListWidget->count(); ++i){
        item = ui->ReplaceInAllPagesListWidget->item(i);
        flags = item->flags();
        if(flags == (flags | Qt::ItemIsUserCheckable)){
            if(ui->checkBox_2->checkState() == Qt::Unchecked){
                item->setCheckState(Qt::Unchecked);
            }
            if(ui->checkBox_2->checkState() == Qt::Checked){
                item->setCheckState(Qt::Checked);
            }
        }
    }
}

/*!
 * \fn GlobalReplaceDialog::rightCheckBoxStateChanged
 * \brief This function handles what happens when states of right side Checkboxes are changed.
 */
void GlobalReplaceDialog::rightCheckBoxStateChanged(){
    QListWidgetItem* item_ = 0;
    Qt::ItemFlags flags;
    int count_=0,count1=0;
    for(int j = 0; j < ui ->ReplaceInAllPagesListWidget->count(); ++j){
        item_ = ui->ReplaceInAllPagesListWidget->item(j);
        flags = item_->flags();
        if(flags == (flags | Qt::ItemIsUserCheckable)){
            count1 ++;
            if(item_->checkState() == Qt::Checked){
                count_ ++;
            }
        }
    }
    if(count_ == count1){
        ui->checkBox_2->setCheckState(Qt::Checked);
    }
    else if( count_ != count1 ){
        ui->checkBox_2->setCheckState(Qt::Unchecked);
    }
    count_ = 0;
    count1=0;
}

/*!
 * \fn GlobalReplaceDialog::disableCheckboxes
 * \brief This function disables the interaction of user with any of the checkboxes in the Global-Replace dialog window if any of the checkboxes in the preview are checked
 * \param flag
 * \param changesCheckedInPreviewMap1
 */
void GlobalReplaceDialog::disableCheckboxes(int flag,QMap<QPair<QString,QString>,QString> *changesCheckedInPreviewMap1)
{
    changesCheckedInPreviewMap = changesCheckedInPreviewMap1;
    ui->groupBox->setDisabled(flag);
    ui->groupBox_2->setDisabled(flag);
    if(flag == 1)
        ui->pushButton_2->setVisible(true);
    if(flag == 0)
        ui->pushButton_2->setVisible(false);
}

/*!
 * \fn GlobalReplaceDialog::on_pushButton_2_clicked
 * \brief This button clears all the checkboxes' states which were checked in preview dialog of global-replace
 */
void GlobalReplaceDialog::on_pushButton_2_clicked()
{
    ui->pushButton_2->setVisible(false);
    ui->groupBox->setDisabled(false);
    ui->groupBox_2->setDisabled(false);
    changesCheckedInPreviewMap->clear();
}

