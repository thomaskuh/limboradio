import {RadioStream} from './radio-stream';

/**
 * Updatable attributes.
 */
export class RadioRequest {

  mode: number;
  threshold: number;
  timeout: number;
  volume: number;
  stream: number;

  streams: RadioStream[];

  wifiName: string;
  wifiPass: string;
}
