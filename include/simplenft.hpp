#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>
#include <eosio/action.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>

using namespace std;
using namespace eosio;


CONTRACT simplenft : public contract {

  public:
    using contract::contract;

  private:

    TABLE nft_listing {
        name            nft_id;
        name            creator; //name of account that created NFT
        uint32_t        created; //seconds since epoch -- current_time_point().sec_since_epoch();
        string          nft_name; //name of nft, string
        string          media; //generally url to image
        string          mtype; //media type
        uint16_t        limit; //maximum limit issued
        uint16_t        minted; //current number minted
        vector<name>    holders; //names of owners by mint index

        auto primary_key() const { return nft_id.value; };
    };

    typedef multi_index <name("nft.listing"), nft_listing> nft_index;

    TABLE nft_holders {
      name              nft_id; 
      name              owner; 
      uint16_t          qty;    // qty held, if zero, just delete row

      // composite id -- ((uint128_t)owner.value << 64)|(uint128_t)nft_id.value;
      auto primary_key() const { return ((uint128_t)owner.value << 64)|(uint128_t)nft_id.value; };
    };

    typedef multi_index <name("nft.holders"), nft_holders> holder_index;

public:

    
    ACTION create(name nft_id, name creator, string nft_name, string media, string mtype, uint16_t limit);
    ACTION update(name nft_id, string nft_name, string media, string mtype);
    ACTION mint(name nft_id, name owner, uint16_t qty);
    ACTION transfer(name nft_id, name from, name to, uint16_t index);
    ACTION zzclearall(name pass);

private:

    void nftcreate(name nft_id, name creator, string nft_name, string media, string mtype, uint16_t limit);
    void nftupdate(name nft_id, string nft_name, string media, string mtype);
    void nftmint(name nft_id, name owner, uint16_t qty);
    void nfttransfer(name nft_id, name from, name to, uint16_t index);
};


