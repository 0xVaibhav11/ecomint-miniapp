// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.13;

import "forge-std/Script.sol";
import {BottleTracker} from "../src/BottleTracker.sol";

contract DeployBottleTracker is Script {
    function run() external returns (BottleTracker tracker) {
        uint256 deployerPrivateKey = vm.envUint("PRIVATE_KEY");
        vm.startBroadcast(deployerPrivateKey);

        tracker = new BottleTracker();

        vm.stopBroadcast();
    }
}
