 // SPDX-License-Identifier: LGPL-2.1-or-later

  /****************************************************************************
   *   Copyright (c) 2002 Jürgen Riegel <juergen.riegel@web.de>               *
   *   Copyright (c) 2023 FreeCAD Project Association                         *
   *                                                                          *
   *   This file is part of FreeCAD.                                          *
   *                                                                          *
   *   FreeCAD is free software: you can redistribute it and/or modify it     *
   *   under the terms of the GNU Lesser General Public License as            *
   *   published by the Free Software Foundation, either version 2.1 of the   *
   *   License, or (at your option) any later version.                        *
   *                                                                          *
   *   FreeCAD is distributed in the hope that it will be useful, but         *
   *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
   *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
   *   Lesser General Public License for more details.                        *
   *                                                                          *
   *   You should have received a copy of the GNU Lesser General Public       *
   *   License along with FreeCAD. If not, see                                *
   *   <https://www.gnu.org/licenses/>.                                       *
   *                                                                          *
   ***************************************************************************/


#ifndef GUI_DIALOG_DLGPREFERENCESIMP_H
#define GUI_DIALOG_DLGPREFERENCESIMP_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStackedWidget>
#include <memory>
#include <FCGlobal.h>

class QAbstractButton;
class QListWidgetItem;
class QTabWidget;

namespace Gui::Dialog {
class PreferencePage;
class Ui_DlgPreferences;

class PreferencesPageItem : public QStandardItem
{
public:
    QWidget* getWidget() const;
    void setWidget(QWidget* widget);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    static constexpr char const* PropertyName = "SettingsPageItem";

private:
    QWidget* _widget = nullptr;
    bool _expanded = false;
};

/**
 * This class implements a dialog containing several preference pages.
 *
 * To append your own page you just have to take note of these points:
 *
 * \li Each preference page can be created by the Qt Designer selecting the "Widget" item
 * in the project dialog.
 *
 * \li To save or load the widgets' settings automatically (e.g. combo boxes, line edits,
 * check boxes, ...) you can make use of the classes inherited from @ref PrefWidget such as:
 * PrefSpinBox, PrefLineEdit, PrefComboBox, PrefListBox, PrefCheckBox, PrefRadioButton and
 * PrefSlider. If you have compiled and installed the library under src/Tools/plugins/widgets
 * to QTDIR/plugins/designer you should see the new category "Preferences".
 * Moreover you have to make sure to have specified the "prefEntry" and "prefPath" properties for each
 * preference widget you have used inside your form in Qt Designer.
 *
 * \li For each widget inside your page - you want to save or load - you have to call
 * \<objectname\>->onSave() or \<objectname\>->onRestore(). The best way to this is either to
 * define the protected slots saveSettings() and loadSettings() in your form and overwrite
 * them in a subclass or define these slots in this subclass directly.
 *
 * See the example below for more details:
 *
 * \code
 *  // This class was created by Qt's uic tool
 *  class MyPrefPage : public QWidget
 *  {
 *  public:
 *    MyPrefPage( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 )
 *    {
 *      myLineEdit->setProperty( "prefEntry", "lineedit" );
 *      myLineEdit->setProperty( "prefPath", "GroupName" );
 *      myCheckBox->setProperty( "prefEntry", "checkbox" );
 *      myCheckBox->setProperty( "prefPath", "GroupName" );
 *      ...
 *    }
 *
 *    PrefLineEdit* myLineEdit;
 *    PrefCheckBox* myCheckBox;
 * };
 * \endcode
 * In the derived class you just have to implement the methods saveSettings() and loadSettings()
 * in the following way:.
 * \code
 *  class MyPrefPageImp : public MyPrefPage
 *  {
 *  public:
 *    MyPrefPageImp( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 )
 *    {
 *    }
 *  protected Q_SLOTS:
 *    void saveSettings()
 *    {
 *      myLineEdit->onSave();
 *      myCheckBox->onSave();
 *    }
 *    void loadSettings();
 *    {
 *      myLineEdit->onRestore();
 *      myCheckBox->onRestore();
 *    }
 * };
 * \endcode
 *
 * \li Now you have to make the widget factory to know your class by adding the line
 * new PrefPageProducer<MyPrefPageImp> (QT_TR_NOOP("My category"));
 *
 * \see PrefWidget
 * \author Werner Mayer, Jürgen Riegel
 */
class GuiExport DlgPreferencesImp : public QDialog
{
    Q_OBJECT

    static constexpr double maxScreenWidthCoveragePercent = 0.8; // maximum % of screen width taken by the dialog
    static constexpr int minVerticalEmptySpace = 100;            // px of vertical space to leave

public:
    static void addPage(const std::string& className, const std::string& group);
    static void removePage(const std::string& className, const std::string& group);
    static void setGroupData(const std::string& group, const std::string& icon, const QString& tip);
    static void getGroupData(const std::string& group, std::string& icon, QString& tip);
    static void reloadSettings();

    static PreferencePage* createPreferencePage(const std::string& pageName, const std::string& groupName);

    explicit DlgPreferencesImp(QWidget* parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());
    ~DlgPreferencesImp() override;

    void accept() override;
    void reject() override;
    void reload();
    void activateGroupPage(const QString& group, int index);
    void activateGroupPageByPageName(const QString& group, const QString& pageName);
    void activeGroupPage(QString& group, int& index) const;

protected:
    void changeEvent(QEvent *e) override;
    void showEvent(QShowEvent*) override;

protected Q_SLOTS:
    void onButtonBoxClicked(QAbstractButton*);
    void onPageSelected(const QModelIndex &index);
    void onStackWidgetChange(int index);

    void onGroupExpanded(const QModelIndex &index);
    void onGroupCollapsed(const QModelIndex &index);

private:
    /** @name for internal use only */
    //@{
    void setupConnections();
    void setupPages();
    void reloadPages();

    PreferencesPageItem* getCurrentPage() const;

    PreferencesPageItem* createGroup(const std::string& groupName);
    void createPageInGroup(PreferencesPageItem* item, const std::string& pageName);

    void applyChanges();
    void showResetOptions();
    void restoreDefaults();
    void restorePageDefaults(PreferencesPageItem* item);
    void restartIfRequired();

    void updatePageDependentWidgets();

    QPixmap loadIconForGroup(const std::string& name) const;

    void addSizeHint(QWidget*);
    int minimumPageWidth() const;
    int minimumDialogWidth(int) const;
    void expandToMinimumDialogWidth();
    //@}

private:
    using TGroupPages = std::pair<std::string, std::list<std::string>>;

    static std::list<TGroupPages> _pages; /**< Name of all registered preference pages */

    QStandardItemModel _model;
    QSize _sizeHintOfPages;

    struct Group {
        std::string iconName;
        QString tooltip;
    };
    static std::map<std::string, Group> _groupMap;
    std::unique_ptr<Ui_DlgPreferences> ui;

    bool invalidParameter;
    bool canEmbedScrollArea;
    bool restartRequired;

    /**< A name for our Qt::UserRole, used when storing user data in a list item */
    static const int GroupNameRole;
    static const int PageNameRole;

    static constexpr char const* GroupNameProperty = "GroupName";
    static constexpr char const* PageNameProperty = "PageName";

    static DlgPreferencesImp* _activeDialog; /**< Defaults to the nullptr, points to the current instance if there is one */
};

} // namespace Gui

#endif // GUI_DIALOG_DLGPREFERENCESIMP_H
