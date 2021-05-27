# Simple NFT
---
Each NFT is registered to a unique owner inside of the nft.listing TABLE.

The vector of "holders" signifies by name format which account name owns each NFT, using the
array index as the signifier of ownership.

The zero position of holders, for example, signifies the first minted owner of that NFT, #1 out of Max Issued.

NFT's are first created, then minted, and then they are able to be transfered.

A maximum of 100 NFT's may be minted at once.  Any given NFT is limited to 1,000 maximum issued.

Contract actions are as follows:
create - Create a new NFT, specifying all data, limit of 1,000 possible NFT's
update - Update name/media data for existing NFT, but does not change ownership
mint - Mints an NFT to owner specified, maximum of 100 mints at a time
transfer - Transfers a single NFT to another owner.

zzclearall - For testing and debugging, will attempt to wipe all data.
Larger TABLE data may take too long to execute zzclearall, in which case modify so that it executes.
This is just a debug / testing function, or a way to close out a contract that is no longer needed.
Operational contracts should consider removing zzclearall entirely.

## Purpose of Simple NFT
----
Other NFT standards may focus on CPU efficiency, but in doing so they require multiple API requests
in order to find how all the required information about an NFT.  This means that certain standards
have very slow website usability, because the API requests are being spammed in the background
just to verify who owns what token.

*Simple NFT solves this issue on the front end, while keeping smart contract CPU under control.*

A single get_table_rows request will return all data related to an NFT entry, including all holders up to 1,000 holders.



## CPU Usage
----
The highest CPU usage I have observed so far, is transfering the zero index of a fully minted 1,000 issued
NFT.  When issuing the #1 of 1,000 NFT into a brand new owner account, it cost less than cpu_usage_us: 8000

Any smart contract interacting with Simple NFT should endeavor to keep its usage under 21500 to 20000 us,
which is more than reasonable for most types of contracts.


