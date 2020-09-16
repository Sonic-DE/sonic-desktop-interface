#ifndef KEYSDATA_H
#define KEYSDATA_H

#include <KCModuleData>

class KeysData : public KCModuleData {
    Q_OBJECT
public:
    KeysData(QObject *parent = nullptr, const QVariantList &args = {});
    bool isDefaults() const override;
};


#endif
