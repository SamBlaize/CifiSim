// In setUpgrades/Talents/Attributes you enter stuff
// Then in main there is iterations and glacium that you can change
// ctrl-f to find them

#include <iostream>
#include <random>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <string>
#include <algorithm>

using namespace std;

struct Enemy
{
    double hp, attack, regen, attackSpeed, critDamage;
    double maxHP, regenTimer, attackTimer, stageLoot, stageLootMulti;
    int stage, stageEnemy, pog, omen;
    int critChance;

    Enemy(int p, int o) : pog(p), omen(o)
    {
        maxHP = hp = 7;
        attack = 1.4;
        regen = 0.0;
        attackSpeed = 6.005;
        stage = 0;
        stageEnemy = 1;
        stageLoot = 1;
        stageLootMulti = 1.074;
    }

    void displayStats()
    {
        cout << "Stage: " << stage << " --- ";
        cout << "HP: " << hp << " -- ";
        cout << "Attack: " << attack << " -- ";
        cout << "Regen: " << regen << " -- ";
        cout << "AS: " << attackSpeed << " -- ";
        cout << "CritChance: " << critChance << " -- ";
        cout << "Crit Damage: " << pog << endl;
    }

    void updateStats()
    {
        maxHP = hp = (7 + 9 * stage) * (1 + (max(0, stage - 49)) * 0.006);
        attack = ((2.4 + 1.4 * stage) * (1 + (max(0, stage - 49)) * 0.006)) * (1 - pog * 0.03);
        regen = (0 + 0.04 * stage) * (1 + (max(0, stage - 49)) * 0.006) * (1 - 0.08 * omen);
        attackSpeed = 6.005 - stage * 0.005;
        critChance = 1000 + 6 * stage;
        critDamage = 1.04 + 0.008 * stage;
        attackTimer = attackSpeed;
    }

    bool advanceEnemy()
    {
        ++stageEnemy;
        hp = maxHP;
        attackTimer = attackSpeed;
        if (stageEnemy > 10)
        {
            stageEnemy = 1;
            ++stage;
            stageLoot *= stageLootMulti;
            updateStats();
            return true;
        }
        else
            return false;
    }

    void triggerRegen()
    {
        int regenTicks = ceil(-regenTimer);
        hp += regen * regenTicks;
        if (hp > maxHP)
            hp = maxHP;
        regenTimer += regenTicks;
    }
};

struct Attributes
{
    int rtk, spa, plfk, timeless, sa, dmtnt, fe, sop, se, pct, kot;

    int pointsSpent()
    {
        int spent = rtk * 1 + spa * 2 + plfk * 3 + timeless * 3 + sa * 1 + dmtnt * 2 + fe * 2 + sop * 3 + se * 2 + pct * 4 + kot * 5;
        return spent;
    }

    int levelRequired()
    {
        return ceil((float)pointsSpent() / 3);
    }
};

struct Talents
{
    int dimc, ca, ua, gb, omen, ll, pog, fm;

    int pointsSpent()
    {
        int spent = dimc + ca + ua + gb + omen + ll + pog + fm;
        return spent;
    }
};

struct Upgrades
{
    int hp, attack, regen, attackSpeed, dmgReduction, block, effect, chargeChance, chargeGain, salvo, level, maxStage;
};

struct Knox
{
    double HP, attack, regen, attackSpeed, dmgReduction, chargeGain, effect;
    int block, chargeChance, revives;
    double maxHP, maxHPPreSouls, attackPreSouls, regenPreSouls, regenTimer, attackTimer, currentCharge, currentTorpedo;
    int salvo, elixirCounter, torpedoCounter, torpedoTriggerAmount, torpedoesToFire;
    double torpedoDamage, finishingMoveMulti, finishingMoveChance;
    int soulChance = 1, souls = 0, soulCap = 100;

    Attributes attributes;
    Talents talents;
    Upgrades upgrades;

    Knox(Attributes attributes, Talents talents, Upgrades upgrades)
    {
        this->attributes = attributes;
        this->talents = talents;
        this->upgrades = upgrades;

        HP = (20 + (2 + (int)(upgrades.hp / 5) * 0.1) * upgrades.hp) * (1 + attributes.rtk * 0.005);
        maxHP = HP;
        maxHPPreSouls = HP;
        attack = (1.2 + (0.06 + (int)(upgrades.attack / 10) * 0.01) * upgrades.attack) * (1 + attributes.rtk * 0.005);
        attackPreSouls = attack;
        regen = (0.05 + (0.03 + (int)(upgrades.regen / 30) * 0.02) * upgrades.regen);
        regenPreSouls = regen;
        dmgReduction = (upgrades.dmgReduction * 0.32 + attributes.plfk * 0.9) / 100;
        block = (8 + upgrades.block * 0.55 + attributes.plfk * 0.8 + attributes.fe * 1) * 100;
        effect = (5 + upgrades.effect * 0.36 + attributes.se * 2 + attributes.plfk * 0.7) * 100;
        chargeChance = (7 + upgrades.chargeChance * 0.25 + attributes.plfk * 0.6 + attributes.se * 1) * 100;
        chargeGain = (0.25 + upgrades.chargeGain * 0.01);
        attackSpeed = (7 - upgrades.attackSpeed * 0.03);
        salvo = 3 + upgrades.salvo;

        soulCap += attributes.dmtnt * 10;
        currentCharge = currentTorpedo = torpedoCounter = torpedoesToFire = 0;
        torpedoTriggerAmount = 100 - (10 * attributes.kot);
        torpedoDamage = 30 * (1 + attributes.pct * 0.08) * (1 + attributes.kot * 0.2);
        finishingMoveMulti = 1 + talents.fm * 0.1;
        finishingMoveChance = effect * 2;
        attackTimer = attackSpeed;
    }

    void updateSoulChance()
    {
        soulChance = (2 + (effect / 100) / 3 + upgrades.maxStage * 0.08f + upgrades.level * 0.14f) * 100;
    }

    void updateSoulStats()
    {
        double soulMulti = 1 + (0.005 * (1 + 0.01 * attributes.sa) * souls);
        double newMaxHP = maxHPPreSouls * soulMulti;
        if (newMaxHP > maxHP)
        {
            HP += newMaxHP - maxHP;
            maxHP = newMaxHP;
        }
        else
            maxHP = newMaxHP;
        attack = attackPreSouls * soulMulti;
        regen = regenPreSouls * soulMulti;
    }

    /*void triggerRegen()
    {
        int regenTicks = ceil(-regenTimer);
        double elixirRegen = (attributes.fe > 0 && elixirCounter > 0) ? regen * (1 + 0.1 * attributes.fe) : regen;
        int elixirTicks = min(elixirCounter, regenTicks);

        HP += elixirRegen * elixirTicks;
        elixirCounter -= elixirTicks;
        HP += regen * (regenTicks - elixirTicks);

        if (HP > maxHP)
            HP = maxHP;

        regenTimer += regenTicks;
    }*/
    void triggerRegen()
    {
        int regenTicks = ceil(-regenTimer);

        // Calculate the elixir regeneration only if elixir is available
        double regenRate = regen;
        int elixirTicks = 0;
        if (attributes.fe > 0 && elixirCounter > 0)
        {
            regenRate *= (1 + 0.1 * attributes.fe);
            elixirTicks = min(elixirCounter, regenTicks);
        }

        HP = min(HP + regenRate * elixirTicks + regen * (regenTicks - elixirTicks), maxHP);  // Directly update HP and clamp to maxHP

        // Update elixir counter and regen timer
        elixirCounter -= elixirTicks;
        regenTimer += regenTicks;
    }

    void lootGain(double& stageLoot, double& totalLoot, mt19937& rng, uniform_int_distribution<int>& dist)
    {
        double lootToAdd = stageLoot;

        if (talents.ll > 0)
        {
            if (dist(rng) < effect)
                lootToAdd *= 1;
        }
        totalLoot += lootToAdd;
    }

    void soulGain(mt19937& rng, uniform_int_distribution<int>& dist, int& soulChanceOnKillCounter, int& averageMaxSoulCapCounter, int& timesReachedMaxSouls, int& stage)
    {
        if (souls < soulCap)
        {
            if (dist(rng) < soulChance)
            {
                ++souls;
                updateSoulStats();
                ++soulChanceOnKillCounter;
            }
            if (souls == soulCap)
            {
                ++timesReachedMaxSouls;
                averageMaxSoulCapCounter = stage;
            }
        }
    }

    void soulCAGain(int& timesReachedMaxSouls, int& averageMaxSoulCapCounter, int& stage, mt19937& rng, uniform_int_distribution<int>& dist)
    {
        if (souls < soulCap && talents.ca > 0)
        {
            if (dist(rng) < (effect * 2.5))
            {
                souls += talents.ca;
                souls = souls > soulCap ? soulCap : souls;
                updateSoulStats();
                if (souls == soulCap)
                {
                    ++timesReachedMaxSouls;
                    averageMaxSoulCapCounter = stage;
                }
            }
        }
    }

    void displayAttributes() const
    {
        cout << "================ HUNTER STATS =================" << endl;
        printf("HP: %.6g - Attack: %.6g - Regen: %.6g\n", HP, attack, regen);

        cout << "DR: " << dmgReduction * 100 << "% - "
            << "Block: " << (float)block / 100 << "% - "
            << "Effect: " << (float)effect / 100 << "%" << endl;
        cout << "Charge Chance: " << (float)chargeChance / 100 << "% - "
            << "Charge Gain: " << chargeGain << " - "
            << "Attack Speed: " << attackSpeed << "s" << endl;
        cout << "Salvo: " << salvo << " - "
            << "Soul Chance: " << (float)soulChance / 100 << "% - "
            << "Soul Cap: " << soulCap << endl;
    }
};

//Change these to read from a json file or something
void setUpgrades(Upgrades& upgrades)
{
    upgrades.hp = 75;
    upgrades.attack = 90;
    upgrades.regen = 80;
    upgrades.dmgReduction = 23;
    upgrades.block = 16;
    upgrades.effect = 15;
    upgrades.chargeChance = 17;
    upgrades.chargeGain = 13;
    upgrades.attackSpeed = 17;
    upgrades.salvo = 2;
    upgrades.level = 14;
    upgrades.maxStage = 69;
}
void setTalents(Talents& talents)
{
    talents.pog = 10;
    talents.ca = 5;
    talents.ll = 0;
    talents.dimc = 0;
    talents.omen = 0;
    talents.gb = 0;
    talents.fm = 0;
}
void setAttributes(Attributes& attributes)
{
    attributes.rtk = 2;
    attributes.spa = 1;
    attributes.plfk = 1;
    attributes.timeless = 5;
    attributes.sa = 1;
    attributes.dmtnt = 9;
    attributes.se = 2;
    attributes.fe = 0;
    attributes.sop = 0;
}

void displayBuild(const Upgrades& upgrades, Talents& talents, Attributes& attributes)
{
    const int labelWidth = 15;

    cout << left << setw(labelWidth) << "== Upgrades =="
        << " | " << setw(labelWidth) << "== Talents =="
        << " | " << setw(labelWidth) << "== Attributes ==" << endl;

    cout << left
        << setw(labelWidth) << "HP: " + to_string(upgrades.hp)
        << " | " << setw(labelWidth) << "DeathIsMy: " + to_string(talents.dimc)
        << " | " << setw(labelWidth) << "Kraken: " + to_string(attributes.rtk) << endl;

    cout << left
        << setw(labelWidth) << "Attack: " + to_string(upgrades.attack)
        << " | " << setw(labelWidth) << "Calypso: " + to_string(talents.ca)
        << " | " << setw(labelWidth) << "Armoury: " + to_string(attributes.spa) << endl;

    cout << left
        << setw(labelWidth) << "Regen: " + to_string(upgrades.regen)
        << " | " << setw(labelWidth) << "UA: " + to_string(talents.ua)
        << " | " << setw(labelWidth) << "Pirate'sLife: " + to_string(attributes.plfk) << endl;

    cout << left
        << setw(labelWidth) << "DR: " + to_string(upgrades.dmgReduction)
        << " | " << setw(labelWidth) << "GhostBullet: " + to_string(talents.gb)
        << " | " << setw(labelWidth) << "Timeless: " + to_string(attributes.timeless) << endl;

    cout << left
        << setw(labelWidth) << "Block: " + to_string(upgrades.block)
        << " | " << setw(labelWidth) << "Omen: " + to_string(talents.omen)
        << " | " << setw(labelWidth) << "SoulAmp: " + to_string(attributes.sa) << endl;

    cout << left
        << setw(labelWidth) << "Effect: " + to_string(upgrades.effect)
        << " | " << setw(labelWidth) << "LuckyLoot: " + to_string(talents.ll)
        << " | " << setw(labelWidth) << "DeadMen: " + to_string(attributes.dmtnt) << endl;

    cout << left
        << setw(labelWidth) << "Charge Ch.: " + to_string(upgrades.chargeChance)
        << " | " << setw(labelWidth) << "PoG: " + to_string(talents.pog)
        << " | " << setw(labelWidth) << "Elixir: " + to_string(attributes.fe) << endl;

    cout << left
        << setw(labelWidth) << "Charge Gain: " + to_string(upgrades.chargeGain)
        << " | " << setw(labelWidth) << "Finishing: " + to_string(talents.fm)
        << " | " << setw(labelWidth) << "SoP: " + to_string(attributes.sop) << endl;

    cout << left
        << setw(labelWidth) << "AS: " + to_string(upgrades.attackSpeed)
        << " | " << setw(labelWidth) << " "
        << " | " << setw(labelWidth) << "SE: " + to_string(attributes.se) << endl;

    cout << left
        << setw(labelWidth) << "Salvo: " + to_string(upgrades.salvo)
        << " | " << setw(labelWidth) << " "
        << " | " << setw(labelWidth) << "PCT: " + to_string(attributes.pct) << endl;

    cout << left
        << setw(labelWidth) << " "
        << " | " << setw(labelWidth) << "Points Spent: "
        << " | " << setw(labelWidth) << "Points Spent: " << endl;

    //int spent = attributes.pointsSpent();
    cout << left
        << setw(labelWidth) << " "
        << " | " << setw(labelWidth) << to_string(talents.pointsSpent())
        << " | " << setw(labelWidth) << to_string(attributes.pointsSpent()) + " -- Lvl " + to_string(attributes.levelRequired()) << endl;
}

int getHPCost(int hp)
{
    return ceil(1 * pow(1.054 + 0.00027 * min(hp, 110), hp));
}
int getAttackCost(int attack)
{
    return ceil(2 * pow(1.068 + 0.00027 * min(attack, 100), attack));
}
int getRegenCost(int regen)
{
    return ceil(4 * pow(1.09 + 0.00027 * min(regen, 70), regen));
}
int getDRCost(int dmgReduction)
{
    return ceil(ceil(2 * pow((dmgReduction - 1) * 0.008 + 1.12, dmgReduction - 1) * 0.9
        * pow(1.2, max(dmgReduction - 1 - 9, 0)) * pow(1.5, max(dmgReduction - 1 - 19, 0))
        * pow(2.0, max(dmgReduction - 1 - 29, 0)) * pow(3.0, max(dmgReduction - 1 - 34, 0))
        * pow(4.0, max(dmgReduction - 1 - 39, 0)) * pow(5.0, max(dmgReduction - 1 - 44, 0))));
}
int getBlockCost(int block)
{
    return ceil(ceil(3 * pow((block - 1) * 0.028 + 1.2, block - 1) * 0.9
        * pow(1.2, max(block - 1 - 9, 0)) * pow(1.5, max(block - 1 - 19, 0))
        * pow(2.0, max(block - 1 - 29, 0)) * pow(3.0, max(block - 1 - 34, 0))
        * pow(4.0, max(block - 1 - 39, 0)) * pow(5.0, max(block - 1 - 44, 0))));
}
int getEffectCost(int effect)
{
    return ceil(ceil(50 * pow((effect - 1) * 0.018 + 1.2, effect - 1) * 0.9
        * pow(1.2, max(effect - 1 - 9, 0)) * pow(1.5, max(effect - 1 - 19, 0))
        * pow(2.0, max(effect - 1 - 29, 0)) * pow(3.0, max(effect - 1 - 34, 0))
        * pow(4.0, max(effect - 1 - 39, 0)) * pow(5.0, max(effect - 1 - 44, 0))));
}
int getChargeChanceCost(int chargeChance)
{
    return ceil(ceil(1 * pow((chargeChance - 1) * 0.016 + 1.18, chargeChance - 1) * 0.9
        * pow(1.05, max(chargeChance - 1 - 9, 0)) * pow(1.05, max(chargeChance - 1 - 19, 0))
        * pow(1.2, max(chargeChance - 1 - 29, 0)) * pow(1.3, max(chargeChance - 1 - 39, 0))
        * pow(1.4, max(chargeChance - 1 - 49, 0)) * pow(1.5, max(chargeChance - 1 - 59, 0))));
}
int getChargeGainCost(int chargeGain)
{
    return ceil(ceil(1 * pow((chargeGain - 1) * 0.025 + 1.35, chargeGain - 1) * 0.9
        * pow(1.05, max(chargeGain - 1 - 9, 0)) * pow(1.05, max(chargeGain - 1 - 19, 0))
        * pow(1.2, max(chargeGain - 1 - 29, 0)) * pow(1.3, max(chargeGain - 1 - 39, 0))
        * pow(1.4, max(chargeGain - 1 - 49, 0)) * pow(1.5, max(chargeGain - 1 - 59, 0))));
}
int getAttackSpeedCost(int attackSpeed)
{
    return ceil(ceil(2 * pow((attackSpeed - 1) * 0.035 + 1.24, attackSpeed - 1) * 0.9
        * pow(1.02, max(attackSpeed - 1 - 9, 0)) * pow(1.05, max(attackSpeed - 1 - 19, 0))
        * pow(1.2, max(attackSpeed - 1 - 29, 0)) * pow(1.3, max(attackSpeed - 1 - 39, 0))
        * pow(1.4, max(attackSpeed - 1 - 49, 0)) * pow(1.5, max(attackSpeed - 1 - 59, 0))
        * pow(1.6, max(attackSpeed - 1 - 69, 0)) * pow(1.7, max(attackSpeed - 1 - 79, 0))
        * pow(1.8, max(attackSpeed - 1 - 89, 0))));
}

void displayUpgradeCosts(Upgrades& upgrades)
{
    cout << "HP " << upgrades.hp << " Cost: " << getHPCost(upgrades.hp) << endl;
    cout << "Attack: " << upgrades.attack << " Cost: " << getAttackCost(upgrades.attack) << endl;
    cout << "Regen: " << upgrades.regen << " Cost: " << getRegenCost(upgrades.regen) << endl;
    cout << "DR: " << upgrades.dmgReduction << " Cost: " << getDRCost(upgrades.dmgReduction) << endl;
    cout << "Block: " << upgrades.block << " Cost: " << getBlockCost(upgrades.block) << endl;
    cout << "Effect: " << upgrades.effect << " Cost: " << getEffectCost(upgrades.effect) << endl;
    cout << "Charge Chance: " << upgrades.chargeChance << " Cost: " << getChargeChanceCost(upgrades.chargeChance) << endl;
    cout << "Charge Gain: " << upgrades.chargeGain << " Cost: " << getChargeGainCost(upgrades.chargeGain) << endl;
    cout << "AttackSpeed: " << upgrades.attackSpeed << " Cost: " << getAttackSpeedCost(upgrades.attackSpeed) << endl;
}

int main()
{
    auto start = chrono::high_resolution_clock::now();
    mt19937 rng(static_cast<unsigned int>(time(0)));
    uniform_int_distribution<int> dist(0, 9999);

    Attributes attributes = {};
    Talents talents = {};
    Upgrades upgrades = {};

    setUpgrades(upgrades);
    setTalents(talents);
    setAttributes(attributes);

    //displayUpgradeCosts(upgrades);

    int glacium = 100000;
    int hpCanBuy = 0;
    int hpCanBuyCosts = 0;
    int attackCanBuy = 0;
    int attackCanBuyCosts = 0;
    int regenCanBuy = 0;
    int regenCanBuyCosts = 0;
    while (true)
    {
        int blah = hpCanBuyCosts;
        blah += getHPCost(upgrades.hp + hpCanBuy);
        if (blah > glacium)
            break;
        ++hpCanBuy;
        hpCanBuyCosts = blah;
    }
    while (true)
    {
        int blah = attackCanBuyCosts;
        blah += getAttackCost(upgrades.attack + attackCanBuy);
        if (blah > glacium)
            break;
        ++attackCanBuy;
        attackCanBuyCosts = blah;
    }
    while (true)
    {
        int blah = regenCanBuyCosts;
        blah += getRegenCost(upgrades.regen + regenCanBuy);
        if (blah > glacium)
            break;
        ++regenCanBuy;
        regenCanBuyCosts = blah;
    }

    cout << "You have " << glacium << " Glacium ready to spend." << endl;
    cout << "For " << hpCanBuyCosts << " Glacium, you can buy " << hpCanBuy << " hp upgrades." << endl;
    cout << "For " << attackCanBuyCosts << " Glacium, you can buy " << attackCanBuy << " attack upgrades." << endl;
    cout << "For " << regenCanBuyCosts << " Glacium, you can buy " << regenCanBuy << " regen upgrades." << endl;

    displayBuild(upgrades, talents, attributes);

    // I'm so sorry for this, I was lazy and wanted this quickly working so here we are
    for (int x = 0; x < 1; ++x)
    {
        if (x == 1)
        {
            cout << "================== HP + " << hpCanBuy << " ==================" << endl;
            upgrades.hp += hpCanBuy;
        }
        else if (x == 2)
        {
            cout << "================ Attack + " << attackCanBuy << " ================" << endl;
            upgrades.hp -= hpCanBuy;
            upgrades.attack += attackCanBuy;
        }
        else if (x == 3)
        {
            cout << "================ Regen + " << regenCanBuy << " ================" << endl;
            upgrades.attack -= attackCanBuy;
            upgrades.regen += regenCanBuy;
        }

        Knox knox(attributes, talents, upgrades);
        knox.updateSoulChance();
        //knox.displayAttributes();
        Enemy enemy(knox.talents.pog, knox.talents.omen);

        int elixirCounter = 0, attackCounter = 0, attackedCounter = 0, blockCounter = 0, spaCounter = 0,
            killCounter = 0, soulChanceOnKillCounter = 0, soulChanceOnStageClearCounter = 0, averageMaxSoulCapCounter = 0,
            averageMaxSoulCap = 0, timesReachedMaxSouls = 0;

        double averageLootCounter = 0;
        double averageStage = 0, bestStage = 0, worstStage = 999,
            averageTime = 0, bestTime = 99999999, worstTime = 0,
            bestlpm = 0, worstlpm = 99999999,
            bestLoot = 0, worstLoot = 99999999;

        double enemyDamage = 1;
        int iterations = 10000;
        int iterationCounter = 0;
        double timelessMulti = 1 + 0.13 * knox.attributes.timeless;
        double luckyLootMulti = 1 + knox.talents.ll * 0.2;

        while (iterationCounter < iterations)
        {
            enemy.stage = 0;
            enemy.updateStats();

            double totalLoot = 0;
            enemy.stageLoot = 1 * timelessMulti;

            knox.elixirCounter = 0;
            attackCounter = attackedCounter = blockCounter = spaCounter = killCounter =
                soulChanceOnKillCounter = soulChanceOnStageClearCounter = averageMaxSoulCapCounter = 0;

            knox.attackTimer = knox.attackSpeed;
            enemy.attackTimer = enemy.attackSpeed;
            knox.regenTimer = 1;
            enemy.regenTimer = 1;

            knox.souls = 0;
            knox.updateSoulStats();
            knox.revives = knox.talents.dimc;
            knox.HP = knox.maxHP;
            enemy.stage = 0;
            enemy.stageEnemy = 1;
            totalLoot = 0;

            while (knox.HP > 0)
            {
                double minTime = min({ knox.attackTimer, enemy.attackTimer });

                if (minTime == knox.attackTimer)
                {
                    knox.regenTimer -= knox.attackTimer;
                    enemy.regenTimer -= knox.attackTimer;
                    enemy.triggerRegen();
                    enemy.attackTimer -= knox.attackTimer;
                    knox.attackTimer = knox.attackSpeed;
                    attackCounter += 1;
                    int shotsToFire = knox.salvo;

                    knox.currentCharge += 0.02 * knox.attributes.pct;
                    //Do extra salvo before charge chance calc
                    //Because it takes another attack to trigger
                    if (knox.currentCharge >= 10)
                    {
                        knox.currentCharge -= 10;
                        shotsToFire *= 2;
                    }
                    if (knox.currentTorpedo >= knox.torpedoTriggerAmount)
                    {
                        knox.currentTorpedo -= knox.torpedoTriggerAmount;
                        knox.torpedoesToFire = 5 + knox.attributes.kot;
                    }
                    if (dist(rng) < knox.chargeChance)
                    {
                        knox.currentCharge += knox.chargeGain;
                        knox.currentTorpedo += knox.chargeGain;
                    }

                    if (knox.talents.gb > 0)
                    {
                        if (dist(rng) < (knox.talents.gb * 5 * 100))
                            ++shotsToFire;
                    }
                    if (knox.attributes.spa > 0)
                    {
                        if (dist(rng) < (knox.attributes.spa * 2 * 100))
                        {
                            spaCounter += 1;
                            shotsToFire += 3;
                        }
                    }
                    // Quick and dirty copy + paste just to get torpedoes working for now because I am lazy
                    // Currently if an enemy survives the torpedo, then the normal attack kills it,
                    // you have to wait until the next attack round for the torpedo to fire. This is not game-accurate, but not too big a deal.
                    // Also if an enemy is really tanky you can hit an enemy with torpedo -> attack -> torpedo, rinse repeat. Not accurate.
                    while (knox.torpedoesToFire > 0)
                    {
                        enemy.hp -= knox.torpedoDamage * knox.attack;
                        --knox.torpedoesToFire;
                        if (enemy.hp <= 0)
                        {
                            ++killCounter;
                            knox.lootGain(enemy.stageLoot, totalLoot, rng, dist);
                            knox.soulGain(rng, dist, soulChanceOnKillCounter, averageMaxSoulCapCounter, timesReachedMaxSouls, enemy.stage);
                            if (enemy.advanceEnemy())
                                knox.soulCAGain(timesReachedMaxSouls, averageMaxSoulCapCounter, enemy.stage, rng, dist);
                        }
                        else
                            break;
                    }
                    while (shotsToFire > 0)
                    {
                        double dam = knox.attack;
                        if (knox.talents.fm > 0 && shotsToFire == 1)
                            if (dist(rng) < knox.finishingMoveChance)
                                dam *= knox.finishingMoveMulti;

                        --shotsToFire;
                        enemy.hp -= dam;

                        if (enemy.hp <= 0)
                        {
                            ++killCounter;
                            knox.lootGain(enemy.stageLoot, totalLoot, rng, dist);
                            knox.soulGain(rng, dist, soulChanceOnKillCounter, averageMaxSoulCapCounter, timesReachedMaxSouls, enemy.stage);
                            if (enemy.advanceEnemy())
                                knox.soulCAGain(timesReachedMaxSouls, averageMaxSoulCapCounter, enemy.stage, rng, dist);
                        }
                    }
                }
                else if (minTime == enemy.attackTimer)
                {
                    knox.regenTimer -= enemy.attackTimer;
                    enemy.regenTimer -= enemy.attackTimer;
                    knox.triggerRegen();
                    knox.attackTimer -= enemy.attackTimer;
                    enemy.attackTimer = enemy.attackSpeed;
                    attackedCounter += 1;
                    enemyDamage = enemy.attack * (1 - knox.dmgReduction);
                    if (dist(rng) < enemy.critChance)
                        enemyDamage *= enemy.critDamage;
                    if (dist(rng) < knox.block)
                    {
                        blockCounter += 1;
                        enemyDamage *= 0.5;

                        if (knox.attributes.fe > 0)
                            knox.elixirCounter = 5;
                        if (knox.attributes.sop > 0)
                        {
                            enemy.hp -= enemyDamage * (0.2 * knox.attributes.sop);
                            if (enemy.hp <= 0)
                            {
                                ++killCounter;
                                knox.lootGain(enemy.stageLoot, totalLoot, rng, dist);
                                knox.soulGain(rng, dist, soulChanceOnKillCounter, averageMaxSoulCapCounter, timesReachedMaxSouls, enemy.stage);
                                if (enemy.advanceEnemy())
                                    knox.soulCAGain(timesReachedMaxSouls, averageMaxSoulCapCounter, enemy.stage, rng, dist);
                            }
                        }
                    }
                    knox.HP -= enemyDamage;
                    if (knox.HP <= 0)
                    {
                        if (knox.revives > 0)
                        {
                            knox.HP = knox.maxHP * 0.8;
                            --knox.revives;
                        }
                        else
                        {
                            if (knox.souls == knox.soulCap)
                                averageMaxSoulCap += averageMaxSoulCapCounter;
                            double currentStage = enemy.stage + ((float)enemy.stageEnemy / 10);
                            bestStage = currentStage > bestStage ? currentStage : bestStage;
                            worstStage = currentStage < worstStage ? currentStage : worstStage;
                            bestLoot = totalLoot > bestLoot ? totalLoot : bestLoot;
                            worstLoot = totalLoot < worstLoot ? totalLoot : worstLoot;
                            break;
                        }
                    }
                }
            }

            int seconds = attackCounter * knox.attackSpeed;
            bestTime = seconds < bestTime ? seconds : bestTime;
            worstTime = seconds > worstTime ? seconds : worstTime;
            double lpm = totalLoot / (seconds / 60);
            bestlpm = lpm > bestlpm ? lpm : bestlpm;
            worstlpm = lpm < worstlpm ? lpm : worstlpm;

            ++iterationCounter;
            averageStage += enemy.stage + (float)enemy.stageEnemy / 10;
            averageTime += seconds + (knox.attackSpeed - knox.attackTimer);
            averageLootCounter += totalLoot;
        }

        double averageLoot = averageLootCounter / iterations;
        int averagerTime = averageTime / iterations;
        cout << "============= " << iterations << " ITERATIONS RUN STATS  =============" << endl;
        cout << "Avg Stage: " << averageStage / iterations << " - "
            << "Best Stage: " << bestStage << " - "
            << "Worst Stage: " << worstStage << endl;
        cout << "Avg Time: " << floor(averagerTime / 60 / 60) << "h " << (averagerTime % 3600) / 60 << "m " << averagerTime % 60 << "s - "
            << "Best Time: " << floor(bestTime / 60 / 60) << "h " << ((int)bestTime % 3600) / 60 << "m " << (int)bestTime % 60 << "s - "
            << "Worst Time: " << floor(worstTime / 60 / 60) << "h " << ((int)worstTime % 3600) / 60 << "m " << (int)worstTime % 60 << "s" << endl;
        cout << "Avg Loot: " << averageLoot << " - "
            << "Best Loot: " << bestLoot << " - "
            << "Worst Loot: " << worstLoot << endl;
        cout << "Avg LPM: " << (averageLoot / (averagerTime / 60)) << " - "
            << "Best LPM: " << bestlpm << " - "
            << "Worst LPM: " << worstlpm << endl;
        double totalLootWithoutXP = averageLoot * 0.9;
        cout << "Avg Glacium: " << totalLootWithoutXP * 0.4047
            << " Avg Aqua: " << totalLootWithoutXP * 0.3385
            << " Avg Tess: " << totalLootWithoutXP * 0.2568
            << " XP: " << averageLoot / 10 * 1.35 << endl;
        cout << "AvgStageMaxSouls: " << (float)averageMaxSoulCap / timesReachedMaxSouls << endl;
        cout << "% of time reached max souls: " << (float)timesReachedMaxSouls / iterations * 100 << "%" << endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Simulation time: " << duration << " milliseconds." << std::endl;

    return 0;
}