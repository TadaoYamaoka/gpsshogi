/* stateInfo.cc
 */
#include "stateInfo.h"
#include "feature.h"
#include "osl/move_probability/stateInfo.h"
#include "osl/checkmate/immediateCheckmateTable.h"
#include "osl/effect_util/sendOffSquare.h"
#include "osl/book/bookInMemory.h"
#include "osl/bits/ptypeTraits.h"
#include "osl/hashKey.h"
#include <iostream>

void gpsshogi::
StateInfo::setThreatmateAuto()
{
  const Move move = last_move >= 0 ? moves[last_move] : Move();
  threatmate_move
    = osl::move_probability::StateInfo::findShortThreatmate(state, move);
}

void gpsshogi::
StateInfo::updateAfterMove()
{
  copy.copyFrom(state);
  const osl::Player turn = state.turn();
  attack_shadow.fill(false);	// need everytime
  pin[BLACK] = state.pin(BLACK);
  pin[WHITE] = state.pin(WHITE);
  move_candidate_exists[BLACK] = king8Info(BLACK).hasMoveCandidate<WHITE>(state);
  move_candidate_exists[WHITE] = king8Info(WHITE).hasMoveCandidate<BLACK>(state);

  checkmate_defender[BLACK] = findCheckmateDefender(state, BLACK);
  checkmate_defender[WHITE] = findCheckmateDefender(state, WHITE);
  updatePinnedGenerals(BLACK);
  updatePinnedGenerals(WHITE);

  sendoffs.clear();
  const Square king = state.kingSquare(alt(turn));
  effect_util::SendOffSquare::find(turn, state, king, sendoffs);

  makePinOfLongPieces();
  for (int d=SHORT8_DIRECTION_MIN; d<=SHORT8_DIRECTION_MAX; ++d) {
    Piece p = state.pieceAt(state.kingMobilityAbs(alt(turn), (Direction)d));
    if (p.isOnBoardByOwner(alt(turn)))
      king8_long_pieces.push_back(p);
  }
  threatened[turn] = state.inCheck(turn)
    ? state.kingPiece(turn) : state.findThreatenedPiece(turn);
  threatened[alt(turn)] = state.findThreatenedPiece(alt(turn));
  BlockLong::updateCache(*this);
  if (last_move < 0 || ! moves[last_move].isNormal()) {
    last_move_ptype5 = PTYPE_EMPTY;
    last_add_effect = PieceMask();
  } else {
    last_move_ptype5 = unpromote(moves[last_move].ptype());
    if (last_move_ptype5 == SILVER)
      last_move_ptype5 = GOLD;
    else if (last_move_ptype5 == KNIGHT)
      last_move_ptype5 = LANCE;
    else if (isMajor(last_move_ptype5))
      last_move_ptype5 = KNIGHT;
    last_add_effect = state.effectedMask(alt(turn))
      & state.effectedChanged(alt(turn));
  }
  PatternCommon::updateCache(*this); // depends on attack_shadow

  {				// new
    possible_threatmate_ptype
      = checkmate::Immediate_Checkmate_Table.dropPtypeMask(king8Info(alt(state.turn())));
    const Square king = state.kingSquare(alt(state.turn()));
    const CArray<Direction,2> directions = {{ UUL, UUR }};
    for (Direction d: directions) {
      Square knight_attack = king - Board_Table.getOffset(state.turn(), d);
      if (state.pieceAt(knight_attack).isEmpty()
	  && ! state.hasEffectAt(alt(state.turn()), knight_attack)) {
	possible_threatmate_ptype |= (1<<(KNIGHT-PTYPE_BASIC_MIN));
	break;
      }	
    }
    for (int p=PTYPE_BASIC_MIN; p<=PTYPE_MAX; ++p) {
      if (possible_threatmate_ptype & (1<<p))
	if (state.hasPieceOnStand(state.turn(), static_cast<Ptype>(p)))
	  p &= ~(1<<p);
    }
  }
  bookmove.fill(Move());
  {
    MoveVector all;
    static const BookInMemory& book = BookInMemory::instance();
    book.find(HashKey(state), all);
    for (size_t i=0; i<bookmove.size(); ++i)
      if (all.size() > i)
	bookmove[i] = all[i];
  }
}

void gpsshogi::
StateInfo::update(osl::Move moved) 
{
  assert(last_move+1<moves.size() && moves[last_move+1] == moved);
  ++last_move;
  progress.update(state, moved); 
  // clear
  pin_by_opposing_sliders.clear();  
  king8_long_pieces.clear();
  long_attack_cache.fill(long_attack_t());
  setThreatmateAuto();		// prior to pattern update
  // 
  updateAfterMove();
}

void gpsshogi::
StateInfo::makePinOfLongPieces()
{
  using namespace osl;
  if (state.inCheck()) 
    return;
  for (int i=0; i<40; ++i) {
    const Piece p = state.pieceOf(i);
    if (!p.isOnBoard() || p.ptype() == KING)
      continue;
    CArray<mask_t,2> long_effect = {{
	state.longEffectAt(p.square(), BLACK),
	state.longEffectAt(p.square(), WHITE),
      }};
    if (long_effect[0].none() || long_effect[1].none()) continue;
    CArray<Piece,2> attack_piece;
    {
      attack_piece[0] = state.findLongAttackAt(p, U);
      attack_piece[1] = state.findLongAttackAt(p, D);
      if (attack_piece[0].isPiece() && attack_piece[1].isPiece()
	  && attack_piece[0].owner() != attack_piece[1].owner()) {
	pin_by_opposing_sliders.push_back(p);
	continue;
      }
    }
    if ((long_effect[0] & mask_t::makeDirect(PtypeFuns<ROOK>::indexMask)).any()
	&& (long_effect[0] & mask_t::makeDirect(PtypeFuns<ROOK>::indexMask)).any())
    {
      attack_piece[0] = state.findLongAttackAt(p, L);
      attack_piece[1] = state.findLongAttackAt(p, R);
      if (attack_piece[0].isPiece() && attack_piece[1].isPiece()
	  && attack_piece[0].owner() != attack_piece[1].owner()) {
	pin_by_opposing_sliders.push_back(p);
	continue;
      }
    }
    if ((long_effect[0] & mask_t::makeDirect(PtypeFuns<BISHOP>::indexMask)).any()
	&& (long_effect[0] & mask_t::makeDirect(PtypeFuns<BISHOP>::indexMask)).any())
    {
      attack_piece[0] = state.findLongAttackAt(p, UL);
      attack_piece[1] = state.findLongAttackAt(p, DR);
      if (attack_piece[0].isPiece() && attack_piece[1].isPiece()
	  && attack_piece[0].owner() != attack_piece[1].owner()) {
	pin_by_opposing_sliders.push_back(p);
	continue;
      }
      attack_piece[0] = state.findLongAttackAt(p, UR);
      attack_piece[1] = state.findLongAttackAt(p, DL);
      if (attack_piece[0].isPiece() && attack_piece[1].isPiece()
	  && attack_piece[0].owner() != attack_piece[1].owner()) {
	pin_by_opposing_sliders.push_back(p);
	continue;
      }
    }	
  }
}

std::pair<osl::Piece,osl::Square> gpsshogi::
StateInfo::findCheckmateDefender(const NumEffectState& state, Player king)
{
  const King8Info king8info = state.king8Info(king);
  const unsigned int spaces = king8info.spaces();
  if (spaces == 0 || (spaces & (spaces-1)))
    return std::make_pair(Piece::EMPTY(), Square());
  const Square sq = state.kingSquare(king)
    + Board_Table.getOffset(king, (Direction)misc::BitOp::bsf(spaces));
  assert(state.pieceAt(sq).isEmpty());
  if (state.countEffect(king, sq) != 2 || ! state.hasEffectAt(alt(king), sq))
    return std::make_pair(Piece::EMPTY(), Square());
  unsigned int drop_candidate = king8info.libertyDropMask();
  drop_candidate &= 0xff00;
  drop_candidate += spaces;
  mask_t drop_ptype=mask_t::makeDirect
    (checkmate::Immediate_Checkmate_Table.dropPtypeMaskOf(drop_candidate));
  while(drop_ptype.any()){
    Ptype ptype=static_cast<Ptype>(drop_ptype.takeOneBit()+PTYPE_BASIC_MIN);
    if (state.hasPieceOnStand(alt(king),ptype))
      return std::make_pair(state.findCheapAttack(king, sq), sq);
  }
  return std::make_pair(Piece::EMPTY(), Square());
}

void
gpsshogi::StateInfo::updatePinnedGenerals(Player owner)
{
  exchange_pins[owner].clear();
  PieceMask attacked = state.piecesOnBoard(owner)
    & state.effectedMask(owner) & state.effectedMask(alt(owner));
  while (attacked.any()) {
    const Piece p = state.pieceOf(attacked.takeOneBit());
    const int a = state.countEffect(alt(owner), p.square());
    const int d = state.countEffect(owner, p.square());
    if (d != a)
      continue;
    const Piece attack_p = state.findCheapAttack(alt(owner), p.square());
    const Piece support = state.findCheapAttack(owner, p.square());
    if (support.ptype() == PAWN || support.ptype() == LANCE)
      continue;
    unsigned int directions = Ptype_Table.getMoveMask(support.ptype());
    while (directions) {
      Direction d = static_cast<Direction>(misc::BitOp::bsf(directions));
      directions &= directions-1;
      Square target = support.square() + Board_Table.getOffset(owner, d);
      if (target == p.square())
	continue;
      Piece tp = state.pieceAt(target);
      if (tp.isEmpty() || tp.isOnBoardByOwner(owner)) {
	assert(state.hasEffectByPiece(support, target));
	if (state.countEffect(owner, target)
	    <= state.countEffect(alt(owner), target)+1
	    - state.hasEffectByPiece(attack_p, target)
	    && ! state.hasEffectIf(support.ptypeO(), target, p.square()))
	  exchange_pins[owner].push_back
	    (PinnedGeneral(support, p, target));
      }
    }
  }
}

bool gpsshogi::operator==(const StateInfo& l, const StateInfo& r)
{
  for (int x=1; x<=9; ++x) {
    for (int y=1; y<=9; ++y) {
      const Square position(x,y);
      if (! (l.pattern_cache[position.index()]
	     == r.pattern_cache[position.index()]))
	return false;
    }
  }
  return HashKey(l.state) == HashKey(r.state)
    && l.moves == r.moves 
    && l.pin_by_opposing_sliders == r.pin_by_opposing_sliders
    && l.king8_long_pieces == r.king8_long_pieces
    && l.last_move == r.last_move && l.threatened == r.threatened
    && l.long_attack_cache == r.long_attack_cache
    && l.attack_shadow == r.attack_shadow
    && l.progress == r.progress
    && l.last_move_ptype5 == r.last_move_ptype5
    && l.last_add_effect == r.last_add_effect
    && l.pin == r.pin && l.threatmate_move == r.threatmate_move
    && l.sendoffs == r.sendoffs
    && l.exchange_pins == r.exchange_pins
    && l.move_candidate_exists == r.move_candidate_exists
    && l.checkmate_defender == r.checkmate_defender
    && HashKey(l.copy) == HashKey(r.copy);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
