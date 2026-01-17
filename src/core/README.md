
+----------------------+
| TorrentMetadata (50) |
|  - pieceLength       |
|  - totalPieces       |
|  - pieceHashes       |
|  - file layout       |
+----------------------+

          │ (read-only)
          ▼

+-----------------------------+
| PeerRegistry (150)          |
|  - peers_: map<PeerId, PeerState>   <-- PeerState per peer
|                             |
|  + addPeer()                |
|  + removePeer()             |
|  + updatePeerPiece()        |
|  + pieceAvailability()      |
+-----------------------------+
          │ (read-only for others)
          ▼
+-----------------------------+
| PieceTracker (200)           |
|  - Tracks state of each piece|
|  - Which pieces are missing/ongoing/have|
+-----------------------------+
          │
          ▼
+-----------------------------+
| PiecePicker (150)            |
|  - Implements rarest-first   |
|  - Selects next piece/block  |
|  - Queries PeerRegistry & PieceTracker |
+-----------------------------+
          │
          ▼
+-----------------------------+
| BlockRequestManager (200)    |
|  - Manages pending block requests |
|  - Handles retries & timeouts    |
|  - Sends requests to peers       |
+-----------------------------+
          │
          ▼
+-----------------------------+
| PieceStorage (150)           |
|  - Writes blocks/pieces to disk|
+-----------------------------+
          │
          ▼
+-----------------------------+
| ProgressReporter (150)       |
|  - Shows progress bar / speed |
|  - Reads PieceTracker / PieceStorage |
+-----------------------------+
          │
          ▼
+-----------------------------+
| PieceManager / Coordinator (80) |
|  - Orchestrates all services     |
|  - Delegates responsibilities    |
+-----------------------------+

