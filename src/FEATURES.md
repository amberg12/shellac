# Search features:
 - Aspiration windows.
 - Negamax with alpha-beta pruning in a fail-soft framework.
 - Quiescence search.
 - Move ordering
   1. TT move
   2. Loud moves ordered by MVV-LVA
   3. One killer move
   4. Promotions.
   5. Quiet moves ordered by history.
   6. Underpromotions.
 - Transposition table
 - PV search
 - Reverse futility pruning.
   - Increased aggressiveness when improving
 - Null move pruning.
 - History heuristics
   - Butterfly tables for quiet moves including maluses
 - Late move reduction
   - Less reduction when captures.
 - Futility pruning.

# Evaluation features:
 - Material Count
 - Piece-Square tables
 - Tapered Evaluation

The evaluation function is taken from PeSTO's evaluation function.