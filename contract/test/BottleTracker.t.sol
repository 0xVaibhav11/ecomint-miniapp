// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.13;

import "forge-std/Test.sol";
import {BottleTracker} from "../src/BottleTracker.sol";

contract BottleTrackerTest is Test {
    BottleTracker tracker;
    address owner = address(0xABCD);
    address verifier = address(0xBEEF);
    uint256 userPk = 0xA11CE;
    address user1;
    address user2 = address(0x2222);

    function setUp() public {
        user1 = vm.addr(userPk);
        vm.prank(owner);
        tracker = new BottleTracker();

        vm.prank(owner);
        tracker.setVerifier(verifier, true);
    }

    function testIncrementAndTotals() public {
        vm.prank(verifier);
        tracker.incrementBottleCount(user1, 5);
        assertEq(tracker.getBottleCount(user1), 5);
        assertEq(tracker.totalBottlesCollected(), 5);

        vm.prank(verifier);
        tracker.incrementBottleCount(user1, 3);
        assertEq(tracker.getBottleCount(user1), 8);
        assertEq(tracker.totalBottlesCollected(), 8);

        vm.prank(verifier);
        tracker.incrementBottleCount(user2, 2);
        assertEq(tracker.getBottleCount(user2), 2);
        assertEq(tracker.totalBottlesCollected(), 10);
    }

    function testSetBottleCountAdjustsTotalsUpAndDown() public {
        vm.startPrank(verifier);
        tracker.incrementBottleCount(user1, 10); // total = 10
        assertEq(tracker.totalBottlesCollected(), 10);

        tracker.setBottleCount(user1, 7); // decrease by 3
        assertEq(tracker.getBottleCount(user1), 7);
        assertEq(tracker.totalBottlesCollected(), 7);

        tracker.setBottleCount(user1, 20); // increase by 13
        assertEq(tracker.getBottleCount(user1), 20);
        assertEq(tracker.totalBottlesCollected(), 20);
        vm.stopPrank();
    }

    function testRedeemMachineSignedIncrement_HappyPathAndReplayProtected() public {
        // Configure machine signer
        uint256 machinePk = 0xB0771E;
        address machine = vm.addr(machinePk);
        vm.prank(owner);
        tracker.setMachine(machine, true);

        bytes32 userId = keccak256(abi.encodePacked("user-123"));
        uint256 amount = 6;
        uint256 nonce = 77;
        uint256 deadline = block.timestamp + 1 days;

        bytes32 dataHash = tracker.hashMachineIncrement(userId, amount, nonce, deadline);
        bytes32 digest = tracker.hashToSign(dataHash);
        (uint8 v, bytes32 r, bytes32 s) = vm.sign(machinePk, digest);
        bytes memory sig = abi.encodePacked(r, s, v);

        tracker.redeemMachineSignedIncrement(userId, amount, nonce, deadline, dataHash, sig);
        assertEq(tracker.getBottleCountById(userId), amount);
        assertEq(tracker.totalBottlesCollected(), amount);
        assertTrue(tracker.usedDigests(digest));

        vm.expectRevert(BottleTracker.AlreadyUsed.selector);
        tracker.redeemMachineSignedIncrement(userId, amount, nonce, deadline, dataHash, sig);
    }

    function testRedeemMachineSignedIncrement_InvalidSignatureOrPayload() public {
        uint256 machinePk = 0xB0771E;
        address machine = vm.addr(machinePk);
        vm.prank(owner);
        tracker.setMachine(machine, true);

        bytes32 userId = keccak256(abi.encodePacked("user-xyz"));
        uint256 amount = 3;
        uint256 nonce = 1;
        uint256 deadline = block.timestamp + 1 days;

        bytes32 dataHash = tracker.hashMachineIncrement(userId, amount, nonce, deadline);
        bytes32 digest = tracker.hashToSign(dataHash);
        // Sign with wrong key
        (uint8 v, bytes32 r, bytes32 s) = vm.sign(0xDEADBEEF, digest);
        bytes memory badSig = abi.encodePacked(r, s, v);
        vm.expectRevert(BottleTracker.NotMachine.selector);
        tracker.redeemMachineSignedIncrement(userId, amount, nonce, deadline, dataHash, badSig);

        // Now sign correctly but send mismatched payload
        (v, r, s) = vm.sign(machinePk, digest);
        bytes memory sig = abi.encodePacked(r, s, v);
        bytes32 wrongDataHash = tracker.hashMachineIncrement(userId, amount + 1, nonce, deadline);
        vm.expectRevert(BottleTracker.InvalidSignature.selector);
        tracker.redeemMachineSignedIncrement(userId, amount, nonce, deadline, wrongDataHash, sig);
    }

    function testRedeemMachineSignedIncrement_Expired() public {
        uint256 machinePk = 0xB0771E;
        address machine = vm.addr(machinePk);
        vm.prank(owner);
        tracker.setMachine(machine, true);

        bytes32 userId = keccak256(abi.encodePacked("user-abc"));
        uint256 amount = 2;
        uint256 nonce = 5;
        uint256 deadline = block.timestamp - 1; // expired

        bytes32 dataHash = tracker.hashMachineIncrement(userId, amount, nonce, deadline);
        bytes32 digest = tracker.hashToSign(dataHash);
        (uint8 v, bytes32 r, bytes32 s) = vm.sign(machinePk, digest);
        bytes memory sig = abi.encodePacked(r, s, v);

        vm.expectRevert(BottleTracker.Expired.selector);
        tracker.redeemMachineSignedIncrement(userId, amount, nonce, deadline, dataHash, sig);
    }

    function testOnlyOwnerAndVerifierRestrictions() public {
        // Only owner can set verifier
        vm.expectRevert(BottleTracker.NotOwner.selector);
        tracker.setVerifier(address(0xCAFE), true);

        // Only verifier can increment
        vm.expectRevert(BottleTracker.NotVerifier.selector);
        tracker.incrementBottleCount(user1, 1);

        // Grant a new verifier and test
        vm.prank(owner);
        tracker.setVerifier(address(0xCAFE), true);

        vm.prank(address(0xCAFE));
        tracker.incrementBottleCount(user1, 4);
        assertEq(tracker.getBottleCount(user1), 4);
        assertEq(tracker.totalBottlesCollected(), 4);
    }
}
