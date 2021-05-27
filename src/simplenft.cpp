#include <simplenft.hpp>

ACTION simplenft::create(name nft_id, name creator, string nft_name, string media, string mtype, uint16_t limit) {
  require_auth(get_self());

  nftcreate(nft_id, creator, nft_name, media, mtype, limit);
}

ACTION simplenft::update(name nft_id, string nft_name, string media, string mtype) {
  require_auth(get_self());

  nftupdate(nft_id, nft_name, media, mtype);
}

ACTION simplenft::mint(name nft_id, name owner, uint16_t qty) {
  require_auth(get_self());

  nftmint(nft_id, owner, qty);
}

ACTION simplenft::transfer(name nft_id, name from, name to, uint16_t index) {
  require_auth(from);

  nfttransfer(nft_id, from, to, index);
}

ACTION simplenft::zzclearall(name pass) {
  require_auth(get_self());

  check(pass.value == name("alpha.omega").value, "Warning!  zzclearall() will delete all contract row data! ");

  nft_index _nft(get_self(), get_self().value);
  auto itr = _nft.begin();
  while (itr != _nft.end()) {
    itr = _nft.erase(itr);
  }

  holder_index _holders( get_self(), get_self().value );
  auto itr2 = _holders.begin();
  while (itr2 != _holders.end()) {
    itr2 = _holders.erase(itr2);
  }
}

//********************** PRIVATE FUNCTIONS

void simplenft::nftcreate(name nft_id, name creator, string nft_name, string media, string mtype, uint16_t limit) {

    nft_index _nft( get_self(), get_self().value );
    auto itr = _nft.find(nft_id.value);

    check(itr == _nft.end(), "nft_id already exists, must be unique index name. ");

    check((nft_name.size() > 0) && (nft_name.size() <= 36), "nft_name cannot be blank and must not exceed 36 characters. ");
    check(media.size() <= 256, "media reference cannot exceed 256 characters. ");
    check(mtype.size() <= 256, "mtype cannot exceed 256 characters. ");
    check(limit > 0, "limit must be greater than 0. ");
    check(limit <= 1000, "limit maximum is 1000. ");

    _nft.emplace( get_self(), [&]( auto& nft_row ) {
      nft_row.nft_id      = nft_id;
      nft_row.creator     = creator;
      nft_row.created     = current_time_point().sec_since_epoch();
      nft_row.nft_name    = nft_name;
      nft_row.media       = media;
      nft_row.mtype       = mtype;
      nft_row.limit       = limit;
      nft_row.minted      = 0;

      print(nft_name, " (", nft_id.to_string(), ") NFT was created!");
    });
}

void simplenft::nftupdate(name nft_id, string nft_name, string media, string mtype) {

    nft_index _nft( get_self(), get_self().value );
    auto itr = _nft.find(nft_id.value);

    check(itr != _nft.end(), "nft_id not found. ");

    check((nft_name.size() > 0) && (nft_name.size() <= 36), "nft_name cannot be blank and must not exceed 36 characters.");
    check(media.size() <= 256, "media reference cannot exceed 256 characters.");
    check(mtype.size() <= 256, "mtype cannot exceed 256 characters.");

    _nft.modify( itr, get_self(), [&]( auto& nft_row ) {

      nft_row.nft_name    = nft_name;
      nft_row.media       = media;
      nft_row.mtype       = mtype;

      print(nft_name, " (", nft_id.to_string(), ") NFT name / media was updated!");
    });
}

void simplenft::nftmint(name nft_id, name owner, uint16_t qty) {

    check( is_account( owner ), "Owner account does not exist. ");

    check(qty > 0, "qty must be at least 1 unit minted. ");

    nft_index _nft( get_self(), get_self().value );
    holder_index _holders( get_self(), get_self().value );
    auto itr = _nft.find(nft_id.value);
    auto itr2 = _holders.find(((uint128_t)owner.value << 64)|(uint128_t)nft_id.value);

    check(itr != _nft.end(), "nft_id does not exist, unable to mint. ");

    if(itr->minted != itr->holders.size()) // if contract error
    { check(false, "Contract error! nftmint - A"); return; } //freeze contract

    check((itr->minted + qty) <= itr->limit, "qty to mint exceeds limit of nft, unable to mint. ");

    check(qty <= 100, "qty may not exceed 100 units minted at once. ");

    //modify nft_index table
    _nft.modify( itr, get_self(), [&]( auto& nft_row ) {

      nft_row.minted = nft_row.minted + qty;

      for (int i = nft_row.minted; i < nft_row.minted + qty; i++) {
        nft_row.holders.push_back(owner);
      }
    });

    //modify holder_index table
    if(itr2 != _holders.end()) { //use current record
      _holders.modify( itr2, get_self(), [&]( auto& holders_row ) {
        holders_row.qty = holders_row.qty + qty;
      });
    } else { //emplace new record
      _holders.emplace( get_self(), [&]( auto& holders_row ) {
        holders_row.nft_id = nft_id;
        holders_row.owner = owner;
        holders_row.qty = qty;
      });
    }

    print("Minted " + to_string(qty) + " units of NFT(" + nft_id.to_string() + ") under account(" + owner.to_string() + "). ");
}

void simplenft::nfttransfer(name nft_id, name from, name to, uint16_t index) {
  check( is_account( to ), "To account does not exist. ");
  check(from.value != to.value, "Transfering to same account is not allowed. ");

  nft_index _nft( get_self(), get_self().value );

  auto itr = _nft.find(nft_id.value);
  
  if(itr->minted != itr->holders.size()) // if contract error
  { check(false, "Contract error! nfttransfer - A"); return; } //freeze contract

  check(itr != _nft.end(), "nft_id does not exist, unable to transfer. ");
  check(index < itr->minted, "index outside of bounds, must be less than minted. ");

  name owner = itr->holders[index];

  holder_index _holders( get_self(), get_self().value );
  auto itr2 = _holders.find(((uint128_t)owner.value << 64)|(uint128_t)nft_id.value);

  if(itr2 == _holders.end())
  { check(false, "Contract error! nfttransfer - B"); return; }

  check(from.value == owner.value, "From specified is not the owner of NFT. ");
  require_auth(owner);
  
  //modify nft_index table
  _nft.modify( itr, get_self(), [&]( auto& nft_row ) {
    nft_row.holders[index] = to;
  });

  //reduce qty for holder or delete
  if( (itr2->qty - 1) == 0 ) { //delete record
    _holders.erase(itr2);
  } else { //reduce qty by 1
    _holders.modify( itr2, get_self(), [&]( auto& holders_row ) {
      holders_row.qty = holders_row.qty - 1;
    });
  }

  auto itr3 = _holders.find(((uint128_t)to.value << 64)|(uint128_t)nft_id.value);
  //increase qty for receiver or create
  if(itr3 == _holders.end()) { //create record
      _holders.emplace( get_self(), [&]( auto& holders_row ) {
        holders_row.nft_id = nft_id;
        holders_row.owner = to;
        holders_row.qty = 1;
      });
  } else {
    _holders.modify( itr3, get_self(), [&]( auto& holders_row ) {
      holders_row.qty = holders_row.qty + 1;
    });
  }

  print("Transferred 1 unit of NFT(" + nft_id.to_string() + ") from account(" + from.to_string() + ") to account(" + to.to_string() + "). ");
}

EOSIO_DISPATCH(simplenft, (create)(update)(mint)(transfer)(zzclearall))
