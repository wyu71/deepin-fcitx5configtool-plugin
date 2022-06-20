/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "layoutselector.h"
#include "dbusprovider.h"
#include "keyboardlayoutwidget.h"
#include "layoutmodel.h"
#include "ui_layoutselector.h"
#include <QDBusPendingCallWatcher>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStringListModel>
#include <QX11Info>
#include <fcitx-utils/i18n.h>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace kcm {

LayoutSelector::LayoutSelector(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent)
    , ui_(std::make_unique<Ui::LayoutSelector>())
    , dbus_(dbus)
    , layoutProvider_(new LayoutProvider(dbus, this))
{
    ui_->setupUi(this);
    ui_->layoutComboBox->setVisible(false);
    ui_->languageComboBox->setVisible(false);
    ui_->variantComboBox->setVisible(false);

    ui_->languageComboBox->setModel(layoutProvider_->languageModel());
    ui_->layoutComboBox->setModel(layoutProvider_->layoutModel());
    ui_->variantComboBox->setModel(layoutProvider_->variantModel());

    connect(ui_->languageComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::languageComboBoxChanged);
    connect(ui_->layoutComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::layoutComboBoxChanged);
    connect(ui_->variantComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::variantComboBoxChanged);

    if (QX11Info::isPlatformX11()) {
        keyboardLayoutWidget_ = new KeyboardLayoutWidget(this);
        keyboardLayoutWidget_->setMinimumSize(QSize(360, 130));
        keyboardLayoutWidget_->setContentsMargins(0,-20,0,0);
        keyboardLayoutWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //keyboardLayoutWidget_->resize(this->size());
    }
}

LayoutSelector::~LayoutSelector() {}

LayoutSelector *LayoutSelector::selectLayout(QWidget *parent, DBusProvider *dbus,
                             const QString &title, const QString &layout)
{
    auto mainLayout = new QVBoxLayout(parent);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    auto layoutSelector = new LayoutSelector(dbus, parent);
    mainLayout->addWidget(layoutSelector);
    layoutSelector->setLayout(layout, "");

    return layoutSelector;
}

void LayoutSelector::setLayout(const QString &layout, const QString &variant)
{
    ui_->layoutComboBox->setCurrentIndex(layoutProvider_->layoutIndex(layout));
    if (!layoutProvider_->loaded()) {
        preSelectLayout_ = layout;
        preSelectVariant_ = variant;
        return;
    }

    preSelectLayout_.clear();
    preSelectVariant_.clear();
}

QPair<QString, QString> LayoutSelector::layout() const
{
    return QPair<QString, QString> ();
}

void LayoutSelector::languageComboBoxChanged()
{
}

void LayoutSelector::layoutComboBoxChanged()
{
    if (ui_->layoutComboBox->currentIndex() < 0) {
        return;
    }

    layoutProvider_->setVariantInfo(
        ui_->layoutComboBox->currentData(LayoutInfoRole)
            .value<FcitxQtLayoutInfo>());
}

void LayoutSelector::variantComboBoxChanged()
{
    if (!keyboardLayoutWidget_) {
        return;
    }

    auto layout = ui_->layoutComboBox->currentData().toString();
    auto variant = ui_->variantComboBox->currentData().toString();
    if (layout.isEmpty()) {
        keyboardLayoutWidget_->setVisible(false);
    } else {
        keyboardLayoutWidget_->setKeyboardLayout(layout, variant);
        keyboardLayoutWidget_->setVisible(true);
    }
}

} // namespace kcm
} // namespace fcitx
