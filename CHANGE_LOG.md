# CHANGE LOG

`date:` 10/19/2024  `version:` 0.0.1  

### changes:
- Tifc serves as entry point that contains application event loop.  

- Canvas is a central entity that connects all modules together.
  (Maybe refactored to extract camera or other things to sep files)  

- Display adjusts responsively to a changes of the terminal window.  

- Input handles mouse events.  
  (keyboard input requires attention)  

- Intoduced a component structure that contains `sparse_t`'s  
  for each component type.

- Introduced behaviors structure that contains `sparse_t`'s  
  for each behavior that satisfies input events and rendering.  
  Each entity type can register its own behavior.  
  There is a component called behavior that contais an index  
  that points to a behavior selected by an entity.

- Intoduced a frame entity that serves as a virtual textual pane.  
  frames indices are stored in canvas struct in dynamic array.  
  This is only way to iterate over them because  
  `sparse_t` has no iterate functionality yet. \(Work in progress\)

### todos:
 - camera is defined and implemented in canvas.c  
   currently dont uses behavior system.  
   __questions__:  
    - Is it necessary to implement it in terms of behaviors  
      and make it a distinct entity by itself?

 - Input mode for text.

 - Figure out how to handle programmed action keys.

 - Resize mode for frames.

 - Parenting/nesting of the frames.

---
