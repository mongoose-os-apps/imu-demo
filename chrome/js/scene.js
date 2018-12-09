"use strict";

var Scene = {
  canvas:      0,
  renderer:    0,
  scene:       0,
  camera:      0,
  hemiLight:   0,
  cube:        0,

  addCamera: function() {
    self.camera = new THREE.PerspectiveCamera(30, canvas.scrollWidth/canvas.scrollHeight, 0.1, 1000);
    self.camera.position.x = 1.4;
    self.camera.position.y = -1.5;
    self.camera.position.z = 1.5;
    self.camera.up = new THREE.Vector3(0, 0, 1);
    self.camera.lookAt(new THREE.Vector3(0.2, 0.2, 0.2));
  },

  addLights: function() {
    self.hemiLight = new THREE.HemisphereLight(0xffffff, 0xffffff, 0.6);
    self.hemiLight.color.setHSL(0.6, 1, 0.6);
    self.hemiLight.groundColor.setHSL(0.095, 1, 0.75);
    self.hemiLight.position.set(0, 100, 0);
    self.scene.add(hemiLight);
  },

  addCube: function() {
    var material = new THREE.MeshPhongMaterial({color: 0xffff00,
      specular: 0xa0a0a0,
      shininess: 15,
      vertexColors: THREE.FaceColors,
      shading: THREE.FlatShading
      });

    var materials = [
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Right.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Left.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Back.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Front.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Top.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Bottom.png')
      }),
    ];
    material = new THREE.MeshFaceMaterial(materials);
    self.cube = new THREE.Mesh(new THREE.BoxGeometry(1.0, 0.6, 0.3), material);
    self.cube.position.x = 0.2;
    self.cube.position.y = 0.2;
    self.cube.position.z = 0.2;
    self.scene.add(cube);
  },

  init: function(dom_elem) {
    self.canvas = document.getElementById(dom_elem);
    self.renderer = new THREE.WebGLRenderer({ canvas: self.canvas });
    self.renderer.setSize(self.canvas.scrollWidth, self.canvas.scrollHeight);
    self.scene = new THREE.Scene();

    this.addCamera();
    this.addLights();
    this.addCube();

    // Render function.
    var render = function () {
      requestAnimationFrame(render);
      renderer.render(self.scene, self.camera);
    };
    render();
  }
};

