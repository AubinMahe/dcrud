package org.hpms.mw.distcrud.sample2;

import java.nio.ByteBuffer;
import java.util.Random;

import org.hpms.mw.distcrud.SerializerHelper;
import org.hpms.mw.distcrud.Shareable;

import javafx.geometry.Bounds;
import javafx.scene.paint.Color;
import javafx.scene.shape.Ellipse;
import javafx.scene.shape.Rectangle;
import javafx.scene.shape.Shape;

abstract class ShareableShape extends Shareable {

   protected static final double MOVE = 1.0;

   protected static Random _Random = new Random();

   protected String _name;
   protected Shape  _shape;
   protected double _dx = +MOVE;
   protected double _dy = +MOVE;

   public ShareableShape( int classId ) {
      super( classId );
   }

   public ShareableShape( int classId, String name, Shape shape ) {
      super( classId );
      _name  = name;
      _shape = shape;
      _shape.setUserData( this );
   }

   @SuppressWarnings("unchecked")
   public <T extends Shape> T getShape() {
      return (T)_shape;
   }

   public void setName( String name ) {
      _name = name;
   }

   public String getName() {
      return _name;
   }

   public abstract void moveIt( Bounds bounds );
}

final class ShareableRect extends ShareableShape {

   public static final int CLASS_ID = 1;

   public ShareableRect() {
      super( CLASS_ID );
   }

   public ShareableRect( String name, Rectangle shape ) {
      super( CLASS_ID, name, shape );
   }

   @Override
   public void set( Shareable source ) {
      final ShareableRect src = (ShareableRect)source;
      this._name  = src._name;
      this._shape = src._shape;
   }

   @Override
   public void serialize( ByteBuffer target ) {
      SerializerHelper.putString( _name, target );
      final Rectangle r = getShape();
      target.putDouble( r.getX());
      target.putDouble( r.getY());
      target.putDouble( r.getWidth());
      target.putDouble( r.getHeight());
      SerializerHelper.putFxColor((Color)_shape.getFill()  , target, Color.WHITESMOKE );
      SerializerHelper.putFxColor((Color)_shape.getStroke(), target, Color.DARKGRAY );
   }

   @Override
   public void unserialize( ByteBuffer source ) {
      _name = SerializerHelper.getString( source );
      final Rectangle r = new Rectangle();
      r.setUserData( this );
      r.setX     ( source.getDouble());
      r.setY     ( source.getDouble());
      r.setWidth ( source.getDouble());
      r.setHeight( source.getDouble());
      r.setFill  ( SerializerHelper.getFxColor( source ));
      r.setStroke( SerializerHelper.getFxColor( source ));
      _shape = r;
   }

   @Override
   public void moveIt( Bounds bounds ) {
      final double    areaMaxX    = bounds.getWidth();
      final double    areaMaxY    = bounds.getHeight();
      final Rectangle rect        = (Rectangle)_shape;
      final double    shapeWidth  = rect.getWidth();
      final double    shapeHeight = rect.getHeight();
      /* */ double    x           = rect.getX();
      /* */ double    y           = rect.getY();
      /* */ boolean   outOfBounds;
      do {
         outOfBounds = false;
         x += _Random.nextDouble()*_dx;
         y += _Random.nextDouble()*_dy;
         if( x < 0 ) {
            outOfBounds = true;
            _dx = +MOVE;
         }
         else if( x+shapeWidth > areaMaxX ) {
            outOfBounds = true;
            _dx = -MOVE;
         }
         if( y < 0 ) {
            outOfBounds = true;
            _dy = +MOVE;
         }
         else if( y+shapeHeight > areaMaxY ) {
            outOfBounds = true;
            _dy = -MOVE;
         }
      } while( outOfBounds );
      rect.setX( x );
      rect.setY( y );
   }
}

final class ShareableEllipse extends ShareableShape {

   public static final int CLASS_ID = 2;

   public ShareableEllipse() {
      super( CLASS_ID );
   }

   public ShareableEllipse( String name, Ellipse shape ) {
      super( CLASS_ID, name, shape );
   }

   @Override
   public void set( Shareable source ) {
      final ShareableEllipse src = (ShareableEllipse)source;
      this._name  = src._name;
      this._shape = src._shape;
   }

   @Override
   public void serialize( ByteBuffer target ) {
      SerializerHelper.putString( _name, target );
      final Ellipse e = getShape();
      target.putDouble( e.getCenterX());
      target.putDouble( e.getCenterY());
      target.putDouble( e.getRadiusX());
      target.putDouble( e.getRadiusY());
      SerializerHelper.putFxColor((Color)_shape.getFill()  , target, Color.WHITESMOKE );
      SerializerHelper.putFxColor((Color)_shape.getStroke(), target, Color.DARKGRAY );
   }

   @Override
   public void unserialize( ByteBuffer source ) {
      _name  = SerializerHelper.getString( source );
      final Ellipse e = new Ellipse();
      e.setUserData( this );
      e.setCenterX( source.getDouble());
      e.setCenterY( source.getDouble());
      e.setRadiusX( source.getDouble());
      e.setRadiusY( source.getDouble());
      e.setFill   ( SerializerHelper.getFxColor( source ));
      e.setStroke ( SerializerHelper.getFxColor( source ));
      _shape = e;
   }

   @Override
   public void moveIt( Bounds bounds ) {
      final double  areaMaxX = bounds.getWidth();
      final double  areaMaxY = bounds.getHeight();
      final Ellipse ellipse  = (Ellipse)_shape;
      final double  radiusX  = ellipse.getRadiusX();
      final double  radiusY  = ellipse.getRadiusY();
      /* */ double  x        = ellipse.getCenterX();
      /* */ double  y        = ellipse.getCenterY();
      /* */ boolean outOfBounds;
      do {
         outOfBounds = false;
         x += _Random.nextDouble()*_dx;
         y += _Random.nextDouble()*_dy;
         if( x-radiusX < 0 ) {
            outOfBounds = true;
            _dx = +MOVE;
         }
         else if( x+radiusX > areaMaxX ) {
            outOfBounds = true;
            _dx = -MOVE;
         }
         if( y-radiusY < 0 ) {
            outOfBounds = true;
            _dy = +MOVE;
         }
         else if( y+radiusY > areaMaxY ) {
            outOfBounds = true;
            _dy = -MOVE;
         }
      } while( outOfBounds );
      ellipse.setCenterX( x );
      ellipse.setCenterY( y );
   }
}
