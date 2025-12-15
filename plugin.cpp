static std::unordered_map<FormID, std::array<float, 4>> effectsMap;

void BuildMap() {
    auto ingredients = TESDataHandler::GetSingleton()->GetFormArray<IngredientItem>();
    for (auto* item : ingredients) {
        std::array<float, 4>& mags = effectsMap[item->GetFormID()];
        mags.fill(0.0f);
        for (size_t i = 0; i < item->effects.size(); ++i) {
            if (auto* effect = item->effects[i]; effect) {
                // if effect has no magnitude: send Duration instead (as negative just so we can identify it as such
                mags[i] = effect->GetMagnitude()
                              ? effect->GetMagnitude()
                              : (effect->GetDuration() ? -static_cast<float>(effect->GetDuration()) : 0.0f);
            }
        }
    }
}

void Inject(BSFixedString menuName) {
    auto ui = UI::GetSingleton();
    if (!ui) return;

    GPtr<IMenu> menu = ui->GetMenu(menuName);
    if (!menu || !menu->uiMovie) {
        return;
    }

    // Alchemy menu is shared with all crafting menus
    auto movie = menu->uiMovie;
    GFxValue Menu;
    if (!movie->GetVariable(&Menu, "_root.Menu")) return;
    GFxValue subTypeName;
    if (!Menu.GetMember("_subtypeName", &subTypeName)) return;
    std::string type = subTypeName.GetString();
    if (type != "Alchemy") return;

    // we cache the ingredients and their effects
    if (effectsMap.empty()) BuildMap();
    if (effectsMap.empty()) return; // precaution

    GFxValue _root;
    movie->GetVariable(&_root, "_root");
    GFxValue data;
    movie->CreateObject(&data);
    for (const auto& [formID, effects] : effectsMap) {
        GFxValue magsArray;
        movie->CreateArray(&magsArray);
        magsArray.SetArraySize(4);
        for (int i = 0; i < 4; i++) {
            magsArray.SetElement(i, effects[i]);
        }
        data.SetMember(std::to_string(formID).c_str(), magsArray);
    }
    _root.SetMember("ingredientMags", data);
    
    GFxValue args[2];
    args[0] = GFxValue("IngMags");
    args[1] = GFxValue(8543);
    _root.Invoke("createEmptyMovieClip", nullptr, args, 2);
    if (movie->GetVariable(&_root, "_root.IngMags")) {
        GFxValue args[1];
        args[0] = GFxValue("ingredientmags_inject.swf");
        _root.Invoke("loadMovie", nullptr, args, 1);
    }
}

class MyEventSink : public RE::BSTEventSink<MenuOpenCloseEvent> {
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
        if (event->menuName == CraftingMenu::MENU_NAME && event->opening) {
            Inject(event->menuName);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

void OnDataLoad() { UI::GetSingleton()->AddEventSink(new MyEventSink()); }

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);

    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            OnDataLoad();
        };
    });

    return true;
}