import skse;

class IngMags extends MovieClip {

    public static var instance;

    public var Menu:MovieClip;
    public var ItemList:MovieClip;
    public var ItemInfo:MovieClip;

    function IngMags() {
        IngMags.instance = this;
    }

    function onLoad() {
        Menu = _parent._parent.Menu;
        ItemList = Menu.ItemList;
        ItemInfo = Menu.ItemInfo;
        duckPunch();
    }

    function duckPunch() {
        ItemInfo.IM__SetupItemName = ItemInfo.SetupItemName;
        ItemInfo.SetupItemName = SetupItemName;
    }

    function SetupItemName(aPrevName: String) : Void {
        this = IngMags.instance;
        ItemInfo.IM__SetupItemName(aPrevName);
        // we need to wait for one frame, because
        // ItemInfo.SetupItemName() runs before ItemInfo.LastUpdateObj is updated
        onEnterFrame = function() {
            displayMagnitudes();
            onEnterFrame = null;
        }
    }

    function displayMagnitudes() {
        if (ItemInfo.LastUpdateObj.type === 8 ) { // 8: Inventory.ICT_INGREDIENT
            var magnitudes:Array = _root.ingredientMags[ItemList.selectedEntry.formId];
            if (! magnitudes.length) {
                return;
            }
            for (var i: Number = 0; i < ItemInfo.LastUpdateObj.numItemEffects; i++) {
                if (ItemInfo.LastUpdateObj["itemEffect" + i] != undefined && ItemInfo.LastUpdateObj["itemEffect" + i] != "") {
                    // if it's negative the value is for Duration instead,
                    // but no plan to use it atm
                    if (magnitudes[i] > 0) {
                        ItemInfo["EffectLabel" + i].SetText(ItemInfo.LastUpdateObj["itemEffect" + i] + '  [' + magnitudes[i] + ']');
                    }
                }
            }
        }
    }
}