## Todo list
- [x] Documentation
- [ ] Linkedin post
- [x] Recover tests
- [x] cleanup main.cpp
- [ ] Fix head-to-head collisions, which make the game crash
- [ ] Color switching following mode in main menu
- [ ] Extra text info in start and gameover screens

#### Collision Edge Case (Current Known Risk)
- [ ] Head-to-head snake collision resolves deterministically and does not crash

#### Performance Gates (Integration Level)
- [ ] 95th percentile frame time stays under target during stress scenario (particles + AI + postFX)
- [ ] AI tick time stays under target on heavy arena preset during active gameplay
- [ ] Particle system update time stays bounded with concurrent menu trail emitters

---

### Long Run
- Entity ID recycling/reusing