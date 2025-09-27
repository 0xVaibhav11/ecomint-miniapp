// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.13;

/// @title BottleTracker
/// @notice Tracks number of recycled bottles per user without minting or deploying a token
/// @dev Minimal role-based access: owner can manage verifiers; verifiers can update counts
contract BottleTracker {
    // --- Roles ---
    address public owner;
    mapping(address => bool) public isVerifier;
    mapping(address => bool) public isMachine;

    // --- Storage ---
    mapping(address => uint256) private userToBottleCount;
    mapping(bytes32 => uint256) private idToBottleCount;
    uint256 public totalBottlesCollected;

    // --- Events ---
    event OwnershipTransferred(address indexed previousOwner, address indexed newOwner);
    event VerifierUpdated(address indexed verifier, bool isActive);
    event BottleCountSet(address indexed user, uint256 previousCount, uint256 newCount, address indexed by);
    event BottleCountIncremented(address indexed user, uint256 incrementBy, uint256 newCount, address indexed by);
    event MachineSignedIncrementRedeemed(
        bytes32 indexed userId,
        uint256 amount,
        uint256 newCount,
        bytes32 digest,
        address indexed submittedBy,
        address indexed machineSigner
    );

    // --- Errors ---
    error NotOwner();
    error NotVerifier();
    error ZeroAddress();
    error InvalidSignature();
    error Expired();
    error AlreadyUsed();
    error ZeroAmount();
    error NotMachine();

    // --- Modifiers ---
    modifier onlyOwner() {
        if (msg.sender != owner) revert NotOwner();
        _;
    }

    modifier onlyVerifier() {
        if (!isVerifier[msg.sender]) revert NotVerifier();
        _;
    }

    constructor() {
        owner = msg.sender;
        emit OwnershipTransferred(address(0), msg.sender);
        // Optional: make owner a verifier by default
        isVerifier[msg.sender] = true;
        emit VerifierUpdated(msg.sender, true);
    }

    // --- Owner functions ---
    function transferOwnership(address newOwner) external onlyOwner {
        if (newOwner == address(0)) revert ZeroAddress();
        address previous = owner;
        owner = newOwner;
        emit OwnershipTransferred(previous, newOwner);
    }

    function setVerifier(address verifier, bool active) external onlyOwner {
        if (verifier == address(0)) revert ZeroAddress();
        isVerifier[verifier] = active;
        emit VerifierUpdated(verifier, active);
    }

    function setMachine(address machine, bool active) external onlyOwner {
        if (machine == address(0)) revert ZeroAddress();
        isMachine[machine] = active;
        // reuse same event for simplicity
        emit VerifierUpdated(machine, active);
    }

    // --- Verifier functions ---
    /// @notice Set exact bottle count for a user (e.g., after off-chain verification)
    function setBottleCount(address user, uint256 newCount) external onlyVerifier {
        if (user == address(0)) revert ZeroAddress();
        uint256 prev = userToBottleCount[user];
        userToBottleCount[user] = newCount;

        // Update global total: adjust by delta
        if (newCount >= prev) {
            unchecked {
                totalBottlesCollected += (newCount - prev);
            }
        } else {
            unchecked {
                totalBottlesCollected -= (prev - newCount);
            }
        }

        emit BottleCountSet(user, prev, newCount, msg.sender);
    }

    /// @notice Increment a user's count by a given amount (common path)
    function incrementBottleCount(address user, uint256 amount) external onlyVerifier {
        if (user == address(0)) revert ZeroAddress();
        if (amount == 0) return; // no-op

        uint256 newCount = userToBottleCount[user] + amount;
        userToBottleCount[user] = newCount;

        unchecked {
            totalBottlesCollected += amount;
        }

        emit BottleCountIncremented(user, amount, newCount, msg.sender);
    }

    // --- Machine-signed QR-based increments ---
    // Track used digests to prevent replay across any caller
    mapping(bytes32 => bool) public usedDigests;

    /// @notice Returns the struct hash for a machine-signed increment authorization
    /// @dev Off-chain QR includes (userId, amount, nonce, deadline) and binds to this contract and chain.
    function hashMachineIncrement(bytes32 userId, uint256 amount, uint256 nonce, uint256 deadline)
        public
        view
        returns (bytes32)
    {
        return keccak256(
            abi.encodePacked("BottleTracker:machine", address(this), block.chainid, userId, amount, nonce, deadline)
        );
    }

    /// @notice EIP-191 style Ethereum Signed Message hash of a 32-byte struct hash
    function hashToSign(bytes32 structHash) public pure returns (bytes32) {
        return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", structHash));
    }

    /// @notice Anyone can submit a machine-signed authorization to increment an off-chain userId
    /// @param userId Off-chain identifier (e.g., UUID/email hash)
    /// @param amount Number of bottles to add (> 0)
    /// @param nonce Unique nonce by machine
    /// @param deadline Expiration timestamp
    /// @param dataHash hashMachineIncrement(userId, amount, nonce, deadline)
    /// @param signature ECDSA signature by authorized machine over hashToSign(dataHash)
    function redeemMachineSignedIncrement(
        bytes32 userId,
        uint256 amount,
        uint256 nonce,
        uint256 deadline,
        bytes32 dataHash,
        bytes calldata signature
    ) external {
        if (amount == 0) revert ZeroAmount();
        if (block.timestamp > deadline) revert Expired();

        bytes32 expected = hashMachineIncrement(userId, amount, nonce, deadline);
        if (expected != dataHash) revert InvalidSignature();
        bytes32 digest = hashToSign(dataHash);
        if (usedDigests[digest]) revert AlreadyUsed();

        address machine = _recover(digest, signature);
        if (!isMachine[machine]) revert NotMachine();

        usedDigests[digest] = true;

        uint256 newCount = idToBottleCount[userId] + amount;
        idToBottleCount[userId] = newCount;
        unchecked {
            totalBottlesCollected += amount;
        }

        emit MachineSignedIncrementRedeemed(userId, amount, newCount, digest, msg.sender, machine);
    }

    function _recover(bytes32 digest, bytes memory signature) internal pure returns (address) {
        if (signature.length != 65) return address(0);
        bytes32 r;
        bytes32 s;
        uint8 v;
        assembly {
            r := mload(add(signature, 0x20))
            s := mload(add(signature, 0x40))
            v := byte(0, mload(add(signature, 0x60)))
        }
        if (v < 27) v += 27;
        if (v != 27 && v != 28) return address(0);
        return ecrecover(digest, v, r, s);
    }

    // --- Views ---
    function getBottleCount(address user) external view returns (uint256) {
        return userToBottleCount[user];
    }

    function getBottleCountById(bytes32 userId) external view returns (uint256) {
        return idToBottleCount[userId];
    }
}
