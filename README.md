# Fragmentation and Reassembly
  Given an MTU of the sender, decision should be made whether to fragment the packet or not. Here I intentionally shuffle my fragments and duplicating it from the sender.Provided the mechanism to reassemble the fragments, considering duplicate fragments,on the reciever side.
  
There is a queue(BufferQueue) to manage the rate of fragments being sent and processed.
And an array(ProcessedFragments) to store the future fragments(not an expected fragment) which will be referenced further after receiving the expected fragment for the next expected fragment.

So far,
  client made send infinite packets and the receiver made listening infinite packets, ie) server is made open for further clients.

Yet to add multi-threading for multiple clients.

### How to run:
##### To compile, 
make all
##### To run server
./server
##### To run client
./client

You can have multiple instance running on different terminal to check
