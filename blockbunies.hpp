#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <atomicassets.hpp>
#include <atomicdata.hpp>
#include <stdint.h>
#include <string>

using namespace eosio;
using namespace std;

CONTRACT blockbunies : public contract
{
public:
  using contract::contract;

  blockbunies(eosio::name self):eosio::contract(self)
  {
    crewPower.clear();
    toolPercent.clear();

    crewPower[204519] = 0.2; crewPower[204521] = 0.2; crewPower[204523] = 0.66; crewPower[204523] = 0.66; 
    crewPower[204528] = 2.18; crewPower[204529] = 7.19; crewPower[204531] = 23.72; crewPower[204532] = 78.27;
    crewPower[204535] = 258.29;

    toolPercent[232860] = 1; toolPercent[232861] = 1; toolPercent[232862] = 1; toolPercent[232864] = 2;
    toolPercent[232887] = 4; toolPercent[232892] = 2; toolPercent[232893] = 4; toolPercent[232894] = 2; 
    toolPercent[233274] = 4;
  }

  
  const uint32_t periodReward = 4*60*60;
  const name collection_name = name("blockbunnie1");
  const name schema_name = name("blockbunnie1");

  struct imeta
  {
    name username;
    string collection_name;
    uint64_t template_id;
    uint64_t assets_id;
  };

  struct rewardmachine
  {
    string machine;
    vector<imeta> crew_nfts;
    vector<imeta> tool_nfts;
    float rewardAmount;
    float totalReward;
    uint64_t last_updated;
    uint64_t next_run;
  };

  ACTION banstaker(const name username);
  ACTION stake(const name username, vector<imeta> crew_nfts, vector<imeta> tool_nfts, string memo);
  ACTION unstake(const name username, string memo);
  ACTION claim(const name username, string memo);
  ACTION burn(const name username, uint64_t assets_id, string memo);

  [[eosio::on_notify("atomicassets::transfer")]] void regstaker(
      name from,
      name to,
      vector<uint64_t> asset_ids,
      string memo);

private:

  map<uint64_t, float> crewPower;
  map<uint64_t, uint8_t> toolPercent;

  TABLE staker_t
  {
    name username;
    vector<rewardmachine> stakedNFTs;
    auto primary_key() const { return username.value; }
  };

  TABLE ban_t
  {
    name username;
    string desc;
    auto primary_key() const { return username.value; }
  };

  TABLE admin_t {
    name username;
    string desc;
    auto primary_key() const { return username.value; }
  };

  typedef eosio::multi_index<name("stakers"), staker_t> staker_s;
  typedef eosio::multi_index<name("bannedlist"), ban_t> ban_s;
  typedef eosio::multi_index<name("adminlist"), admin_t> admin_s;

  staker_s stakes = staker_s(get_self(), get_self().value);
  ban_s bans = ban_s(get_self(), get_self().value);
  admin_s admins = admin_s(get_self(), get_self().value);

  void in_contract_transfer(name recipient, vector<uint64_t> assets_id, string msg);

};