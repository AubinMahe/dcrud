package tests.shapes;

import java.nio.ByteBuffer;
import java.util.Random;

import org.hpms.mw.distcrud.ClassID;
import org.hpms.mw.distcrud.Shareable;

import javafx.geometry.Bounds;
import javafx.scene.paint.Color;
import javafx.scene.shape.Ellipse;
import javafx.scene.shape.Rectangle;
import javafx.scene.shape.Shape;

abstract class ShareableShape extends Shareable {

   protected static final double MOVE = 1.0;

   protected static Random _Random = new Random();

   protected /* */ String _name;
   protected final Shape  _shape;
   protected /* */ double _dx = +MOVE;
   protected /* */ double _dy = +MOVE;

   public ShareableShape( ClassID classId, Shape shape ) {
      super( classId );
      _shape = shape;
   }

   public ShareableShape( ClassID classId, String name, Shape shape ) {
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

   public static final ClassID CLASS_ID = new ClassID((byte)1, (byte)1, (byte)1, (byte)1 );

   public ShareableRect() {
      super( CLASS_ID, new Rectangle());
   }

   public ShareableRect( String name, Rectangle shape ) {
      super( CLASS_ID, name, shape );
   }

   @Override
   public void serialize( ByteBuffer target ) {
      org.hpms.mw.distcrud.SerializerHelper.putString( _name, target );
      final Rectangle r = getShape();
      target.putDouble( r.getX());
      target.putDouble( r.getY());
      target.putDouble( r.getWidth());
      target.putDouble( r.getHeight());
      tests.SerializerHelper.putFxColor((Color)_shape.getFill()  , target, Color.WHITESMOKE );
      tests.SerializerHelper.putFxColor((Color)_shape.getStroke(), target, Color.DARKGRAY );
   }

   @Override
   public void unserialize( ByteBuffer source ) {
      _name = org.hpms.mw.distcrud.SerializerHelper.getString( source );
      final Rectangle r = getShape();
      r.setUserData( this );
      final double x = source.getDouble();
      final double y = source.getDouble();
      final double w = source.getDouble();
      final double h = source.getDouble();
      r.setX     ( x );
      r.setY     ( y );
      r.setWidth ( w );
      r.setHeight( h );
      r.setFill  ( tests.SerializerHelper.getFxColor( source ));
      r.setStroke( tests.SerializerHelper.getFxColor( source ));
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
         else if( ( x+shapeWidth ) > areaMaxX ) {
            outOfBounds = true;
            _dx = -MOVE;
         }
         if( y < 0 ) {
            outOfBounds = true;
            _dy = +MOVE;
         }
         else if( ( y+shapeHeight ) > areaMaxY ) {
            outOfBounds = true;
            _dy = -MOVE;
         }
      } while( outOfBounds );
      rect.setX( x );
      rect.setY( y );
   }
}

final class ShareableEllipse extends ShareableShape {

   public static final ClassID CLASS_ID = new ClassID((byte)1, (byte)1, (byte)1, (byte)2 );

   public ShareableEllipse() {
      super( CLASS_ID, new Ellipse());
   }

   public ShareableEllipse( String name, Ellipse shape ) {
      super( CLASS_ID, name, shape );
   }

   @Override
   public void serialize( ByteBuffer target ) {
      org.hpms.mw.distcrud.SerializerHelper.putString( _name, target );
      final Ellipse e = getShape();
      target.putDouble( e.getCenterX());
      target.putDouble( e.getCenterY());
      target.putDouble( e.getRadiusX());
      target.putDouble( e.getRadiusY());
      tests.SerializerHelper.putFxColor((Color)_shape.getFill()  , target, Color.WHITESMOKE );
      tests.SerializerHelper.putFxColor((Color)_shape.getStroke(), target, Color.DARKGRAY );
   }

   @Override
   public void unserialize( ByteBuffer source ) {
      _name  = org.hpms.mw.distcrud.SerializerHelper.getString( source );
      final Ellipse e = getShape();
      e.setUserData( this );
      final double x  = source.getDouble();
      final double y  = source.getDouble();
      final double rx = source.getDouble();
      final double ry = source.getDouble();
      e.setCenterX( x );
      e.setCenterY( y );
      e.setRadiusX( rx );
      e.setRadiusY( ry );
      e.setFill   ( tests.SerializerHelper.getFxColor( source ));
      e.setStroke ( tests.SerializerHelper.getFxColor( source ));
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
         if( ( x-radiusX ) < 0 ) {
            outOfBounds = true;
            _dx = +MOVE;
         }
         else if( ( x+radiusX ) > areaMaxX ) {
            outOfBounds = true;
            _dx = -MOVE;
         }
         if( ( y-radiusY ) < 0 ) {
            outOfBounds = true;
            _dy = +MOVE;
         }
         else if( ( y+radiusY ) > areaMaxY ) {
            outOfBounds = true;
            _dy = -MOVE;
         }
      } while( outOfBounds );
      ellipse.setCenterX( x );
      ellipse.setCenterY( y );
   }
}
