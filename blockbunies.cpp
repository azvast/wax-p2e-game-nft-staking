#include <blockbunies.hpp>

using namespace eosio;
static inline time_point_sec current_time_point_sec() { return time_point_sec(current_time_point()); };


void blockbunies::regstaker(
name from,
name to,
vector<uint64_t> asset_ids,
string memo)
{
  if (from == get_self() || to != get_self())
      return;

  auto it_ban = bans.find(from.value);
  check(it_ban == bans.end(), "You were banned, please see your administrator");      
}

ACTION blockbunies::stake(const name username, vector<imeta> crew_nfts, vector<imeta> tool_nfts, string memo) 
{
  check(has_auth(username), "No authority");
  auto itr = stakes.find(username.value);

  float amount = 0;
  int percent = 0;

  for (uint8_t i = 0; i < crew_nfts.size(); ++i)
  {
      amount += crewPower[crew_nfts[i].template_id];
      percent += toolPercent[tool_nfts[i].template_id];
  }

  amount += amount * percent * 1.0 / 100.0;

  rewardmachine rm;
  rm.crew_nfts = crew_nfts;
  rm.tool_nfts = tool_nfts;
  rm.last_updated = current_time_point_sec().utc_seconds;
  rm.next_run = current_time_point_sec().utc_seconds + periodReward;
  rm.machine = memo;
  rm.rewardAmount = amount;
  rm.totalReward = 0;

  if(itr == stakes.end()) { // add staker
      stakes.emplace(get_self(), [&](auto &v) {
          v.username = username;
          v.stakedNFTs.push_back(rm);

      });
  }
  else { // update staker
      stakes.modify(itr, get_self(), [&](auto &v) {
          v.username = username;
          v.stakedNFTs.push_back(rm);
      });
  }

}


ACTION blockbunies::unstake(const name username, string memo)
{
  check(has_auth(username), "No authority");
  auto itr = stakes.require_find(username.value, "Your are not a staker, please register first");
  vector<uint64_t> total_nfts;

  for (uint8_t i = 0; i < itr->stakedNFTs.size(); i++)
  {
      for (uint8_t j = 0; j < itr->stakedNFTs[i].crew_nfts.size(); j++)
      {
      total_nfts.push_back(itr->stakedNFTs[i].crew_nfts[j].assets_id);
      }

      for (uint8_t j = 0; j < itr->stakedNFTs[i].tool_nfts.size(); j++)
      {
      total_nfts.push_back(itr->stakedNFTs[i].tool_nfts[j].assets_id);
      }
  }
  in_contract_transfer(username, total_nfts, string("Your NFT_staked have been unstaked"));
  stakes.erase(itr);
}



ACTION blockbunies::claim(const name username, string memo)
{
  check(has_auth(username), "No authority");
  auto itr = stakes.require_find(username.value, "Your are not a staker, please register first");

  check(itr != stakes.end(), "You don't have staked NFTs");

  for(auto& item : stakes) {
      for (int i = 0; i < item.stakedNFTs.size(); i ++) {
          if(item.stakedNFTs[i].next_run < current_time_point_sec().utc_seconds) {
              stakes.modify(item, _self, [&](auto &a) {
                  a.stakedNFTs[i].totalReward += a.stakedNFTs[i].rewardAmount;  
                  a.stakedNFTs[i].last_updated += current_time_point_sec().utc_seconds;
                  a.stakedNFTs[i].next_run += a.stakedNFTs[i].last_updated + periodReward;
              });
          }  
      }
  }
}

ACTION blockbunies::banstaker(const name username)
{

}

ACTION burn(const name username, uint64_t assets_id, string memo)
{

}

void blockbunies::in_contract_transfer(name recipient, vector<uint64_t> assets_id, string msg)
{
  auto memo = " withdrawn by " + recipient.to_string();
  action(permission_level{get_self(), name("active")},
          "atomicassets"_n, name("transfer"),
          std::make_tuple(get_self(), recipient, assets_id, memo))
      .send();
}