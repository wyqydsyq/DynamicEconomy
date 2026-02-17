![DynamicEconomy](Assets/de_splash.png "DynamicEconomy")

# DynamicEconomy

## About
Arma Reforger Conflict-compatible dynamic economy system with cash, banking/ATMs and traders/shops.

Originally created to implement banking and traders in my FFA PvPvE game mode [MercOut!](https://reforger.armaplatform.com/workshop/64C4F3E0169E5739-MercOut), released stand-alone for others to use in their own game modes or servers 🙂

## Setup
See included `Arland_Test.ent` world or contents of `Prefabs/DE_Examples` for examples.

### Systems
 - Needs a Systems Config with `DL_LootSystem` and `DE_EconomySystem` enabled e.g. `ConflictSystems.conf` overrides include them or add them to your own config.
 - Configure global settings like default trader margins, cash:supply exchange rate etc on `DE_EconomySystem`. Note that any changes to persisted values like exchange rate will not take effect until a wipe/new save.

### Traders
 - Add `DE_TraderComponent` or `DE_VehicleTraderComponent` to your trader entity (probably a character, make sure to disable their damage manager if you don't want them to be killable!)
 - Configure any settings on the component as needed. By default traders can sell every item in EntityCatalogs of all factions in the world's FactionManager which can be filtered based on regular Arsenal filters or a prefab whitelist for unique traders. Most settings will use global system defaults if left unchanged.

### Banking
 - For ATMs, simply add `DE_ATM.et` prefabs to your world / buildings
 - For Banker characters, add `DE_BankComponent` to desired banker character prefab

## How it works
The primary trader and banker components (`DE_TraderComponent`, `DE_VehicleTraderComponent` and `DE_BankComponent`)
add themselves to a queue in the economy system that is only processed server-side.

Each server system tick will create the associated underlying entity prefabs for each queued component
to make them interactible and add their required components to function properly, then remove it from the queue.
E.g. each entity with a `DE_TraderComponent` gets a `DE_TraderEntity` added as a child and configured according to settings on the component.

Item Traders are based on vanilla Conflict Arsenal logic.
Vehicle Traders are based on vanilla Conflict Vehicle Service logic.
Using other mods that override these vanilla systems directly may cause conflicts!

## Credits
 - Cash stack model: Sebastian Webster - https://sketchfab.com/3d-models/australian-20-bills-d0e6db1b7cca4c89976d118c9b88fab7
 - ATM model: Mehdi Shahsavan - https://sketchfab.com/3d-models/atm-12-mb-11d95f8395e4422b8aa29ea7044d2810

## License
APL - https://www.bohemia.net/community/licenses/arma-public-license