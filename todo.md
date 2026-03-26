## Todo list
- [x] Documentation
- [ ] Linkedin post
- [x] Recover tests
- [x] cleanup main.cpp
- [ ] Fix head-to-head collisions, which make the game crash
- [ ] Color switching following mode in main menu
- [ ] Extra text info in start and gameover screens

#### Rendering and Frame Pipeline
- [ ] Mode2D/Mode3D switch during gameplay preserves render/update coherence
- [ ] Post-processing on/off toggle does not break UI composition order
- [ ] Tunnel animation and particle rendering coexist without state leakage

#### Collision Edge Case (Current Known Risk)
- [ ] Head-to-head snake collision resolves deterministically and does not crash

#### End-to-End Soak and Stability
- [ ] 300-frame deterministic soak in Playing (fixed seed, scripted inputs) produces stable outcomes
- [ ] Repeated state cycling (Menu <-> Playing <-> GameOver) for N loops has no leaks under Valgrind
- [ ] Long-run entity churn (spawn/despawn gameplay objects across rounds) shows no invalid accesses
- [ ] Config reload + gameplay transition sequence remains stable (no crash, no stale pointers)

#### Performance Gates (Integration Level)
- [ ] 95th percentile frame time stays under target during stress scenario (particles + AI + postFX)
- [ ] AI tick time stays under target on heavy arena preset during active gameplay
- [ ] Particle system update time stays bounded with concurrent menu trail emitters

---

### Long Run
- Entity ID recycling/reusing