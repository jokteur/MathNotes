#include "translate.h"
#include "IconsMaterialDesign.h"

Translator build_FR() {
    Translator fr;
    fr.texts["Files"] = "Fichiers";
    fr.texts[ICON_MD_NOTE_ADD " New project"] = ICON_MD_NOTE_ADD " Nouveau projet";
    return fr;
}